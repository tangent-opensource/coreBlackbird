/*
 * Copyright 2011-2016 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "render/attribute.h"
#include "render/camera.h"
#include "render/mesh.h"

#include "subd/subd_patch.h"
#include "subd/subd_patch_table.h"
#include "subd/subd_split.h"

#include "util/util_algorithm.h"
#include "util/util_hash.h"
#include "util/util_math.h"

#ifdef WITH_OPENSUBDIV

#  include <opensubdiv/far/patchMap.h>
#  include <opensubdiv/far/patchTableFactory.h>
#  include <opensubdiv/far/primvarRefiner.h>
#  include <opensubdiv/far/stencilTable.h>
#  include <opensubdiv/far/ptexIndices.h>
#  include <opensubdiv/far/stencilTableFactory.h>
#  include <opensubdiv/far/topologyRefinerFactory.h>

/* specializations of TopologyRefinerFactory for ccl::Mesh */

namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {
namespace Far {
template<>
bool TopologyRefinerFactory<ccl::Mesh>::resizeComponentTopology(TopologyRefiner &refiner,
                                                                ccl::Mesh const &mesh)
{
  const ccl::Mesh::SubdFace *face = mesh.subd_faces.data();
  size_t total_num_corners = 0;

  // uniform
  setNumBaseFaces(refiner, mesh.subd_faces.size());
  for (int i = 0; i < mesh.subd_faces.size(); i++, face++) {
    setNumBaseFaceVertices(refiner, i, face->num_corners);
    total_num_corners += face->num_corners;
  }

  // face-vertex
  setNumBaseVertices(refiner, mesh.verts.size());

  return true;
}

template<>
bool TopologyRefinerFactory<ccl::Mesh>::assignComponentTopology(TopologyRefiner &refiner,
                                                                ccl::Mesh const &mesh)
{
  const ccl::Mesh::SubdFace *face = mesh.subd_faces.data();

  for (int i = 0; i < mesh.subd_faces.size(); i++, face++) {
    IndexArray face_verts = getBaseFaceVertices(refiner, i);

    const int *corner = &mesh.subd_face_corners[face->start_corner];

    for (int j = 0; j < face->num_corners; j++, corner++) {
      face_verts[j] = *corner;
    }
  }

  return true;
}

template<>
bool TopologyRefinerFactory<ccl::Mesh>::assignComponentTags(TopologyRefiner &refiner,
                                                            ccl::Mesh const &mesh)
{
  const ccl::Mesh::SubdEdgeCrease *crease = mesh.subd_creases.data();

  for (int i = 0; i < mesh.subd_creases.size(); i++, crease++) {
    Index edge = findBaseEdge(refiner, crease->v[0], crease->v[1]);

    if (edge != INDEX_INVALID) {
      setBaseEdgeSharpness(refiner, edge, crease->crease * 10.0f);
    }
  }

  // TODO what is this doing here?
  for (int i = 0; i < mesh.verts.size(); i++) {
    ConstIndexArray vert_edges = getBaseVertexEdges(refiner, i);

    if (vert_edges.size() == 2) {
      float sharpness = refiner.getLevel(0).getEdgeSharpness(vert_edges[0]);
      sharpness = ccl::min(sharpness, refiner.getLevel(0).getEdgeSharpness(vert_edges[1]));

      setBaseVertexSharpness(refiner, i, sharpness);
    }
  }

  return true;
}

template<>
bool TopologyRefinerFactory<ccl::Mesh>::assignFaceVaryingTopology(TopologyRefiner &refiner,
                                                                  const ccl::Mesh &mesh)
{
  int total_num_corners = 0;
  for (size_t i = 0; i < mesh.subd_faces.size(); ++i) {
    total_num_corners += mesh.subd_faces[i].num_corners;
  }

  createBaseFVarChannel(refiner, total_num_corners);

  int off = 0;
  for (int face = 0; face < mesh.subd_faces.size(); ++face) {
    setNumBaseFaceVertices(refiner, face, mesh.subd_faces[face].num_corners);

    auto index_array = getBaseFaceFVarValues(refiner, face);
    for (auto it = index_array.begin(); it != index_array.end(); ++it, ++off) {
      *it = off;
    }
  }

  return true;
}

template<>
void TopologyRefinerFactory<ccl::Mesh>::reportInvalidTopology(TopologyError /*err_code*/,
                                                              char const * /*msg*/,
                                                              ccl::Mesh const & /*mesh*/)
{
}
} /* namespace Far */
} /* namespace OPENSUBDIV_VERSION */
} /* namespace OpenSubdiv */

CCL_NAMESPACE_BEGIN

using namespace OpenSubdiv;

/* struct that implements OpenSubdiv's vertex interface */

template<typename T> struct OsdValue {
  T value;

  OsdValue()
  {
  }

  void Clear(void * = 0)
  {
    memset(&value, 0, sizeof(T));
  }

  void AddWithWeight(OsdValue<T> const &src, float weight)
  {
    value += src.value * weight;
  }
};

template<> void OsdValue<uchar4>::AddWithWeight(OsdValue<uchar4> const &src, float weight)
{
  for (int i = 0; i < 4; i++) {
    value[i] += (uchar)(src.value[i] * weight);
  }
}

/* class for holding OpenSubdiv data used during tessellation */

class OsdData {
 public:
  explicit OsdData(ccl::Mesh *mesh_) : mesh{mesh_}
  {
    if (mesh->subdivision_type == Mesh::SUBDIVISION_CATMULL_CLARK) {
      initialize_adaptive(Sdc::SchemeType::SCHEME_CATMARK);
    }

    if (mesh->subdivision_type == Mesh::SUBDIVISION_LINEAR) {
      initialize_adaptive(Sdc::SchemeType::SCHEME_BILINEAR);
    }
  }

  const Far::PatchTable *get_patch_table() const
  {
    return patch_table.get();
  }

  const Far::PatchMap *get_patch_map() const
  {
    return patch_map.get();
  }

  const vector<OsdValue<float3>> &get_vertices() const
  {
    return verts;
  }

  int get_face_index(int ptex_index) const
  {
    return ptex_to_face_index_map[ptex_index];
  }

  int get_face_size() const
  {
    return Sdc::SchemeTypeTraits::GetRegularFaceSize(refiner->GetSchemeType());
  }

  const Mesh *get_mesh() const
  {
    return mesh;
  }

  const Far::TopologyRefiner *get_refiner() const
  {
    return refiner.get();
  }

 private:
  void initialize_adaptive(Sdc::SchemeType scheme_type)
  {
    /* scheme type type and options */
    Sdc::Options scheme_options;
    scheme_options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER);
    scheme_options.SetFVarLinearInterpolation(Sdc::Options::FVAR_LINEAR_ALL);

    /* create topology refiner */
    Far::TopologyRefinerFactory<Mesh>::Options refiner_options(scheme_type, scheme_options);
    auto refiner_ptr = Far::TopologyRefinerFactory<Mesh>::Create(*mesh, refiner_options);
    refiner = std::unique_ptr<Far::TopologyRefiner>(refiner_ptr);

    /* adaptive refinement */
    int max_isolation_level = calculate_max_isolation();
    Far::TopologyRefiner::AdaptiveOptions adaptive_options(max_isolation_level);
    adaptive_options.considerFVarChannels = true;
    refiner->RefineAdaptive(adaptive_options);

    // Ptex Index Face -> Coarse Index Face mapping
    // In this code path Osd works with quadrangular patches and it 0 topology level(base_level)
    // is quadrangulated version of the input mesh. To find original coarse face index, where
    // ptex patches were generated from, we build an offset table:
    // * input face is a quad then 1 quadrangular patch is generated
    // * input face is not a quad then num_corners quadrangular patches are generated.
    const Far::TopologyLevel &base_level = refiner->GetLevel(0);
    int face_size = Sdc::SchemeTypeTraits::GetRegularFaceSize(refiner->GetSchemeType());

    // worst case, if all input faces have to be quadrangulated
    ptex_to_face_index_map.reserve(base_level.GetNumFaces() * face_size);

    for (int coarse_face = 0; coarse_face < base_level.GetNumFaces(); ++coarse_face) {
      int num_base_vertices = base_level.GetFaceVertices(coarse_face).size();
      int num_ptex_faces = (num_base_vertices == face_size) ? 1 : num_base_vertices;
      for (int i = 0; i < num_ptex_faces; ++i) {
        ptex_to_face_index_map.push_back(coarse_face);
      }
    }
    ptex_to_face_index_map.shrink_to_fit();

    /* create patch table */
    Far::PatchTableFactory::Options patch_options;
//    patch_options.generateAllLevels = true;
    patch_options.SetEndCapType(Far::PatchTableFactory::Options::ENDCAP_GREGORY_BASIS);
    patch_table = std::unique_ptr<Far::PatchTable>(
        Far::PatchTableFactory::Create(*refiner, patch_options));

    /* subdivide vertices */
    int num_refiner_verts = refiner->GetNumVerticesTotal();
    int num_local_points = patch_table->GetNumLocalPoints();
    verts.resize(num_refiner_verts + num_local_points);
    for (size_t i = 0; i < mesh->verts.size(); i++) {
      verts[i].value = mesh->verts[i];
    }

    Far::PrimvarRefiner primvar_refiner{*refiner};
    OsdValue<float3> *src = verts.data();
    for (int i = 0; i < refiner->GetMaxLevel(); i++) {
      OsdValue<float3> *dest = src + refiner->GetLevel(i).GetNumVertices();
      primvar_refiner.Interpolate(i + 1, src, dest);
      src = dest;
    }

    if (num_local_points) {
      patch_table->ComputeLocalPointValues(&verts[0], &verts[num_refiner_verts]);
    }

    /* create patch map */
    patch_map = std::make_unique<Far::PatchMap>(*patch_table);
  }

 public:
  void subdivide_attribute(Attribute &attr)
  {
    Far::PrimvarRefiner primvar_refiner(*refiner);

    if (attr.element == ATTR_ELEMENT_VERTEX) {
      int num_refiner_verts = refiner->GetNumVerticesTotal();
      int num_local_points = patch_table->GetNumLocalPoints();

      attr.resize(num_refiner_verts + num_local_points);
      attr.flags |= ATTR_FINAL_SIZE;

      char *src = attr.buffer.data();

      for (int i = 0; i < refiner->GetMaxLevel(); i++) {
        char *dest = src + refiner->GetLevel(i).GetNumVertices() * attr.data_sizeof();

        if (attr.same_storage(attr.type, TypeDesc::TypeFloat)) {
          primvar_refiner.Interpolate(i + 1, (OsdValue<float> *)src, (OsdValue<float> *&)dest);
        }
        else if (attr.same_storage(attr.type, TypeFloat2)) {
          primvar_refiner.Interpolate(i + 1, (OsdValue<float2> *)src, (OsdValue<float2> *&)dest);
        }
        else {
          primvar_refiner.Interpolate(i + 1, (OsdValue<float4> *)src, (OsdValue<float4> *&)dest);
        }

        src = dest;
      }

      if (num_local_points) {
        if (attr.same_storage(attr.type, TypeDesc::TypeFloat)) {
          patch_table->ComputeLocalPointValues(
              (OsdValue<float> *)&attr.buffer[0],
              (OsdValue<float> *)&attr.buffer[num_refiner_verts * attr.data_sizeof()]);
        }
        else if (attr.same_storage(attr.type, TypeFloat2)) {
          patch_table->ComputeLocalPointValues(
              (OsdValue<float2> *)&attr.buffer[0],
              (OsdValue<float2> *)&attr.buffer[num_refiner_verts * attr.data_sizeof()]);
        }
        else {
          patch_table->ComputeLocalPointValues(
              (OsdValue<float4> *)&attr.buffer[0],
              (OsdValue<float4> *)&attr.buffer[num_refiner_verts * attr.data_sizeof()]);
        }
      }
    }
    else if (attr.std == ATTR_STD_UV) {
      // size is equal to number of
      auto num_frav_values_total = refiner->GetNumFVarValuesTotal();

      attr.resize(num_frav_values_total);
      attr.flags |= ATTR_FINAL_SIZE;

      char *src = attr.buffer.data();

      for (int level = 0; level < refiner->GetMaxLevel(); ++level) {
        const Far::TopologyLevel &topology_level = refiner->GetLevel(level);

        auto num_fvar_values = topology_level.GetNumFVarValues();
        char *dest = src + num_fvar_values * attr.data_sizeof();

        if (attr.same_storage(attr.type, TypeDesc::TypeFloat)) {
          primvar_refiner.InterpolateFaceVarying(
              level + 1, (OsdValue<float> *)src, (OsdValue<float> *&)dest);
        }
        else if (attr.same_storage(attr.type, TypeFloat2)) {
          primvar_refiner.InterpolateFaceVarying(
              level + 1, (OsdValue<float2> *)src, (OsdValue<float2> *&)dest);
        }
        else {
          primvar_refiner.InterpolateFaceVarying(
              level + 1, (OsdValue<float4> *)src, (OsdValue<float4> *&)dest);
        }

        src = dest;
      }
    }
  }

  int calculate_max_isolation()
  {
    /* loop over all edges to find longest in screen space */
    const Far::TopologyLevel &level = refiner->GetLevel(0);
    Transform objecttoworld = mesh->subd_params->objecttoworld;
    Camera *cam = mesh->subd_params->camera;

    float longest_edge = 0.0f;

    for (size_t i = 0; i < level.GetNumEdges(); i++) {
      Far::ConstIndexArray verts = level.GetEdgeVertices(i);

      float3 a = mesh->verts[verts[0]];
      float3 b = mesh->verts[verts[1]];

      float edge_len;

      if (cam) {
        a = transform_point(&objecttoworld, a);
        b = transform_point(&objecttoworld, b);

        edge_len = len(a - b) / cam->world_to_raster_size((a + b) * 0.5f);
      }
      else {
        edge_len = len(a - b);
      }

      longest_edge = max(longest_edge, edge_len);
    }

    /* calculate isolation level */
    int isolation = (int)(log2f(max(longest_edge / mesh->subd_params->dicing_rate, 1.0f)) + 1.0f);

    return min(isolation, 10);
  }

 private:
  Mesh *mesh;
  vector<OsdValue<float3>> verts;

  std::unique_ptr<Far::TopologyRefiner> refiner;
  std::unique_ptr<Far::PatchTable> patch_table;
  std::unique_ptr<Far::PatchMap> patch_map;

  ccl::vector<int> ptex_to_face_index_map;
};

/* ccl::Patch implementation that uses OpenSubdiv for eval */

class OsdPatch final : public Patch {
 public:
  OsdPatch(const OsdData *data, const Far::PatchParam &patch_param) : osd_data{data}
  {
    patch_index = patch_param.GetFaceId();

    // ptex index -> input face
    const Mesh::SubdFace& face = get_face();

    shader = face.shader;
    from_ngon = face.num_corners != osd_data->get_face_size();
  }

  OsdPatch(const OsdData* data, int ptex_index) : osd_data{data}
  {
    patch_index = ptex_index;

    const Mesh::SubdFace& face = get_face();
    shader = face.shader;
    from_ngon = face.num_corners != osd_data->get_face_size();
  }

  void eval(float3 *P, float3 *dPdu, float3 *dPdv, float3 *N, float u, float v) const override
  {
    // search for a patch within top level ptex patch
    const Far::PatchMap *patch_map = osd_data->get_patch_map();
    auto ptex_index = get_ptex_index();
    const Far::PatchTable::PatchHandle *handle = patch_map->FindPatch(ptex_index, u, v);
    assert(handle);

    const Far::PatchTable *patch_table = osd_data->get_patch_table();

    float p_weights[20]{}, du_weights[20]{}, dv_weights[20]{};
    patch_table->EvaluateBasis(*handle, u, v, p_weights, du_weights, dv_weights);

    float3 du{}, dv{};
    if (P) {
      *P = make_float3(0.0f, 0.0f, 0.0f);
    }
    du = make_float3(0.0f, 0.0f, 0.0f);
    dv = make_float3(0.0f, 0.0f, 0.0f);

    Far::ConstIndexArray cv = patch_table->GetPatchVertices(*handle);
    for (int i = 0; i < cv.size(); i++) {
      float3 p = osd_data->get_vertices()[cv[i]].value;

      if (P) {
        *P += p * p_weights[i];
      }

      du += p * du_weights[i];
      dv += p * dv_weights[i];
    }

    if (dPdu) {
      *dPdu = du;
    }

    if (dPdv) {
      *dPdv = dv;
    }

    if (N) {
      *N = cross(du, dv);

      float t = len(*N);
      *N = (t != 0.0f) ? *N / t : make_float3(0.0f, 0.0f, 1.0f);
    }
  }

  int get_ptex_index() const
  {
    return get_patch_index();
  }

  const Mesh::SubdFace &get_face() const
  {
    auto ptex_index = get_patch_index();
    auto face_index = osd_data->get_face_index(ptex_index);
    assert(face_index < osd_data->get_mesh()->subd_faces.size());
    return osd_data->get_mesh()->subd_faces[face_index];
  }

 private:
  const OsdData *osd_data;
};

//
// Process of adaptive subdivision is separated into following steps:
// * adaptive subdivision - osd patch generation
// * tessellation:
//    - splitting to sub-patches
//    - dicing - uniform subdivision in dicing camera space
//

static void osd_tessellate(Mesh *mesh, DiagSplit *split)
{
  OsdData osd_data{mesh};
  bool need_packed_patch_table = false;

  // OsdPatch implementation abstracts OpenSubdiv patches. OsdPatch is created for each Ptex Patch
  //
  // we don't create OsdPatch of each OpenSubdiv patch.
  // Instead, during eval, lookup for Osd patches happens. Vector of OsdPatches represents
  // base level that comes from the refiner.

  const Far::TopologyRefiner* osd_refiner = osd_data.get_refiner();
  const Far::TopologyLevel& osd_base_level = osd_refiner->GetLevel(0);
  const int face_size = Sdc::SchemeTypeTraits::GetRegularFaceSize(osd_refiner->GetSchemeType());

  ccl::vector<OsdPatch> patches;
  patches.reserve(osd_base_level.GetNumFaces() * face_size); // worse case scenario
  for(int base_face = 0, ptex_index=0; base_face < osd_base_level.GetNumFaces(); ++base_face) {
    int num_face_vertices = osd_base_level.GetFaceVertices(base_face).size();
    int num_ptex_faces = (num_face_vertices == face_size) ? 1 : num_face_vertices;
    for (int i = 0; i < num_ptex_faces; ++i, ++ptex_index) {
      patches.emplace_back(&osd_data, ptex_index);
    }
  }

  // split patches
  split->split(patches);

  // attributes
  for (Attribute &attr : mesh->subd_attributes.attributes) {
    osd_data.subdivide_attribute(attr);
    need_packed_patch_table = true;
  }

  // build mesh data indices
  const Far::PatchTable* osd_patch_table = osd_data.get_patch_table();

  //
  // construct vertex data lookup
  //

  mesh->subd_patch_vertex_data_indices.resize(osd_patch_table->GetNumPtexFaces() * 4, 0);
  mesh->subd_patch_corner_data_indices.resize(osd_patch_table->GetNumPtexFaces() * 4, 0);

  for(int face = 0, patch_index = 0; face < osd_base_level.GetNumFaces(); ++face) {
    auto face_vertices = osd_base_level.GetFaceVertices(face);

    if(face_vertices.size() == 4) {
      mesh->subd_patch_vertex_data_indices[patch_index * 4 + 0] = face_vertices[0];
      mesh->subd_patch_vertex_data_indices[patch_index * 4 + 1] = face_vertices[1];
      mesh->subd_patch_vertex_data_indices[patch_index * 4 + 2] = face_vertices[2];
      mesh->subd_patch_vertex_data_indices[patch_index * 4 + 3] = face_vertices[3];

      ++patch_index;
    } else {
      // special case of n-gon being
      const Mesh::SubdFace& subd_face = mesh->subd_faces[face];

      auto child_faces = osd_base_level.GetFaceChildFaces(face);
      for(int i = 0; i < child_faces.size(); ++i) {
        int child_face = child_faces[i];

        auto child_face_vertices = osd_refiner->GetLevel(1).GetFaceVertices(child_face);
        assert(child_face_vertices.size() == 4);

        int m;
        m = mod(i + 0, subd_face.num_corners);
        mesh->subd_patch_vertex_data_indices[patch_index * 4 + 0] = face_vertices[m];

        m = mod(i + 1, subd_face.num_corners);
        mesh->subd_patch_vertex_data_indices[patch_index * 4 + 1] = face_vertices[m];

        m = 2;
        mesh->subd_patch_vertex_data_indices[patch_index * 4 + 2] = osd_base_level.GetNumVertices() + child_face_vertices[2];

        m = mod(i - 1, subd_face.num_corners);
        mesh->subd_patch_vertex_data_indices[patch_index * 4 + 3] = face_vertices[m];
        ++patch_index;
      }
    }
  }

  // packed patch table
  if (need_packed_patch_table) {
    delete mesh->patch_table;
    mesh->patch_table = new PackedPatchTable;
    mesh->patch_table->pack(osd_data.get_patch_table());
  }
}

#endif  // WITH_OPENSUBDIV

// build in tessellation when osd is not available
static void ccl_linear_tessellate(Mesh *mesh, DiagSplit *split)
{
  auto verts = mesh->verts.data();
  auto subd_faces = mesh->subd_faces;
  auto subd_face_corners = mesh->subd_face_corners.data();
  const AttributeSet &subd_attributes = mesh->subd_attributes;

  const Attribute *attr_vN = subd_attributes.find(ATTR_STD_VERTEX_NORMAL);
  const float3 *vN = (attr_vN) ? attr_vN->data_float3() : nullptr;

  size_t num_faces = subd_faces.size();

  /* count patches */
  size_t num_patches = 0;
  for (size_t f = 0; f < num_faces; f++) {
    const Mesh::SubdFace &face = subd_faces[f];
    num_patches += face.is_quad() ? 1 : face.num_corners;
  }

  vector<LinearQuadPatch> linear_patches(num_patches);
  auto *patch = linear_patches.data();

  for (size_t f = 0; f < num_faces; f++) {
    const Mesh::SubdFace &face = subd_faces[f];

    if (face.is_quad()) {
      float3 *hull = patch->hull;
      float3 *normals = patch->normals;

      patch->patch_index = face.ptex_offset;
      patch->from_ngon = false;

      for (int i = 0; i < 4; i++) {
        hull[i] = verts[subd_face_corners[face.start_corner + i]];
      }

      if (face.smooth) {
        for (int i = 0; i < 4; i++) {
          normals[i] = vN[subd_face_corners[face.start_corner + i]];
        }
      }
      else {
        float3 N = face.normal(mesh);
        for (int i = 0; i < 4; i++) {
          normals[i] = N;
        }
      }

      swap(hull[2], hull[3]);
      swap(normals[2], normals[3]);

      patch->shader = face.shader;
      patch++;
    }
    else {
      /* ngon */
      float3 center_vert = make_float3(0.0f, 0.0f, 0.0f);
      float3 center_normal = make_float3(0.0f, 0.0f, 0.0f);

      float inv_num_corners = 1.0f / float(face.num_corners);
      for (int corner = 0; corner < face.num_corners; corner++) {
        center_vert += verts[subd_face_corners[face.start_corner + corner]] * inv_num_corners;
        center_normal += vN[subd_face_corners[face.start_corner + corner]] * inv_num_corners;
      }

      for (int corner = 0; corner < face.num_corners; corner++) {
        float3 *hull = patch->hull;
        float3 *normals = patch->normals;

        patch->patch_index = face.ptex_offset + corner;
        patch->from_ngon = true;

        patch->shader = face.shader;

        hull[0] = verts[subd_face_corners[face.start_corner + mod(corner + 0, face.num_corners)]];
        hull[1] = verts[subd_face_corners[face.start_corner + mod(corner + 1, face.num_corners)]];
        hull[2] = verts[subd_face_corners[face.start_corner + mod(corner - 1, face.num_corners)]];
        hull[3] = center_vert;

        hull[1] = (hull[1] + hull[0]) * 0.5;
        hull[2] = (hull[2] + hull[0]) * 0.5;

        if (face.smooth) {
          normals[0] =
              vN[subd_face_corners[face.start_corner + mod(corner + 0, face.num_corners)]];
          normals[1] =
              vN[subd_face_corners[face.start_corner + mod(corner + 1, face.num_corners)]];
          normals[2] =
              vN[subd_face_corners[face.start_corner + mod(corner - 1, face.num_corners)]];
          normals[3] = center_normal;

          normals[1] = (normals[1] + normals[0]) * 0.5;
          normals[2] = (normals[2] + normals[0]) * 0.5;
        }
        else {
          float3 N = face.normal(mesh);
          for (int i = 0; i < 4; i++) {
            normals[i] = N;
          }
        }

        patch++;
      }
    }
  }

  // split to sub-patches
  split->split(linear_patches);

  // attributes
}

static void ccl_tessellate(Mesh *mesh, DiagSplit *split)
{
  // TODO: implement catmull
  ccl_linear_tessellate(mesh, split);
}

void Mesh::tessellate(DiagSplit *split)
{
  if (subdivision_type == SUBDIVISION_NONE) {
    return;
  }

#ifdef WITH_OPENSUBDIV
  osd_tessellate(this, split);
#else
  ccl_tessellate(this, split);
#endif
}

CCL_NAMESPACE_END
