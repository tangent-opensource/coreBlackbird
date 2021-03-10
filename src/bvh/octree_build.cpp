/*
 * Copyright 2021 Tangent Animation
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
 *
 * Author: Sergen Eren
 */

#include "bvh/octree_build.h"

#include "render/image.h"
#include "render/object.h"

CCL_NAMESPACE_BEGIN

/* Constructor / Destructor */
OCTBuild::OCTBuild() : depth(3), octree_root(nullptr)
{
}

OCTBuild::~OCTBuild()
{
}

int OCTBuild::get_num_nodes()
{
  return (pow(8, depth + 1) - 1) / 7;
}

void OCTBuild::init_octree()
{
  octree_root = new OCTNode;
  octree_root->parent = nullptr;
  octree_root->idx = 0;
  build_root_rec(octree_root, depth, 0);
}

void OCTBuild::update_octree(const vector<ImageHandle *> &handles)
{
  for (int i = 0; i < handles.size(); i++) {

    ImageHandle *handle = handles[i];
    const ImageMetaData &metadata = handle->metadata();

    if (metadata.width == 0 || metadata.height == 0 || metadata.depth == 0) {
      continue;
    }
    if (metadata.depth > 1) {
      octree_root->bbox.grow(metadata.world_bounds);

      octree_root->num_volumes++;
      octree_root->vol_indices[i] = handle->svm_slot();
      octree_root->min_extinction = ccl::min(octree_root->min_extinction, metadata.min);
      octree_root->max_extinction = ccl::max(octree_root->max_extinction, metadata.max);
    }
  }

  update_root_rec(octree_root, handles);
}

void OCTBuild::reset_octree()
{
  clear_root_rec(octree_root);
}

void OCTBuild::build_root_rec(OCTNode *root, int depth, int parent_idx)
{
  root->bbox = BoundBox(BoundBox::empty);
  memset(root->vol_indices, 0, sizeof(int) * 1024);
  root->depth = depth;

  if (depth > 0) {
    for (int i = 0; i < 8; i++) {
      root->children[i] = new OCTNode;
      root->children[i]->parent = root;
      root->children[i]->idx = (parent_idx * 8) + (i+1);
      root->children[i]->parent_idx = parent_idx;

      build_root_rec(root->children[i], depth - 1, root->children[i]->idx);

      root->child_idx[i] = root->children[i]->idx;
    }

    root->has_children = true;
  }
}

vector<OCTNode *> OCTBuild::flatten_octree()
{
  vector<OCTNode *> ret;

  ret.reserve(get_num_nodes());
  for (int i = 0; i < get_num_nodes(); i++) {
    ret.push_back(nullptr);
  }

  if (octree_root) {
    ret[0] = octree_root;
    flatten_root_rec(ret, octree_root);
  }

  return ret;
}

/* Recursive Functions*/

void OCTBuild::update_root_rec(OCTNode *node, const vector<ImageHandle *> &handles)
{
  if (node->has_children) {
    for (int i = 0; i < 8; i++) {

      node->children[i]->bbox = divide_bbox(node->bbox, i);

      int vol_idx = 0;
      for (int slot = 0; slot < handles.size(); slot++) {

        ImageHandle *handle = handles[slot];
        const ImageMetaData &metadata = handle->metadata();

        if (metadata.width == 0 || metadata.height == 0 || metadata.depth == 0) {
          continue;
        }

        if (metadata.depth > 1) {
          if (metadata.world_bounds.intersects(node->children[i]->bbox)) {

            node->children[i]->num_volumes++;
            node->children[i]->vol_indices[vol_idx] = handle->svm_slot();
            node->children[i]->min_extinction = ccl::min(node->children[i]->min_extinction,
                                                         metadata.min);
            node->children[i]->max_extinction = ccl::max(node->children[i]->max_extinction,
                                                         metadata.max);
            vol_idx++;
          }
        }
      }

      update_root_rec(node->children[i], handles);
    }
  }
}

void OCTBuild::clear_root_rec(OCTNode *node)
{
  if (!node) {
    return;
  }

  if (node->has_children) {
    for (int i = 0; i < 8; i++) {
      clear_root_rec(node->children[i]);
    }
  }

  node->max_extinction = make_float3(0.0f);
  node->min_extinction = make_float3(FLT_MAX);
  node->num_volumes = 0;
  node->bbox = BoundBox(BoundBox::empty);
}

void OCTBuild::flatten_root_rec(vector<OCTNode *> &vec, OCTNode *root)
{
  if (root->has_children) {
    for (int i = 0; i < 8; i++) {
      vec[root->children[i]->idx] = root->children[i];
      flatten_root_rec(vec, root->children[i]);
    }
  }
}

CCL_NAMESPACE_END
