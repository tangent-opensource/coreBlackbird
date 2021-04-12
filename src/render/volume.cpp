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

#include "render/volume.h"
#include "render/attribute.h"
#include "render/geometry.h"
#include "render/object.h"
#include "render/scene.h"

#include "bvh/octree_build.h"

#include "util/util_foreach.h"
#include "util/util_progress.h"
#include "util/util_task.h"

CCL_NAMESPACE_BEGIN

VolumeManager::VolumeManager()
{
  need_update = true;
#ifdef WITH_OCTREE
  octree_builder = new OCTBuild;
  octree_builder->init_octree();
#endif
}

VolumeManager::~VolumeManager()
{
#ifdef WITH_OCTREE
  octree_builder->reset_octree();
#endif
}

void VolumeManager::device_update(DeviceScene *dscene, Scene *scene, Progress &progress)
{
  if (!need_update)
    return;

  volume_objects.clear();

  foreach (Object *object, scene->objects) {
    if (object->geometry->has_volume) {
      volume_objects.push_back(object);
    }
  }
}

void VolumeManager::device_update_octree(DeviceScene *dscene, Scene *scene, Progress &progress)
{
  if (!need_update || volume_objects.size() == 0)
    return;
#ifdef WITH_OCTREE
  octree_builder->reset_octree();

  /* Parallel update volume object world bounding boxes */
  static const int OBJECTS_PER_TASK = 32;
  parallel_for(blocked_range<size_t>(0, volume_objects.size(), OBJECTS_PER_TASK),
               [&](const blocked_range<size_t> &r) {
                 for (size_t i = r.begin(); i != r.end(); i++) {
                   Object *ob = volume_objects[i];
                   Transform tfm = ob->tfm;
                   foreach (Attribute &attr, ob->geometry->attributes.attributes) {
                     if (attr.element == ATTR_ELEMENT_VOXEL) {
                       ImageHandle &handle = attr.data_voxel();
                       handle.update_world_bbox(tfm);
                     }
                   }
                 }
               });

  if (progress.get_cancel())
    return;

  octree_builder->update_octree(scene->objects);

  vector<OCTNode *> host_vector = octree_builder->flatten_octree();
  KernelOCTree *k_tree_root = dscene->octree_nodes.alloc(octree_builder->get_num_nodes());

  foreach (OCTNode *node, host_vector) {
    k_tree_root[node->idx].max_extinction = node->max_extinction;
    k_tree_root[node->idx].min_extinction = node->min_extinction;
    k_tree_root[node->idx].has_children = node->has_children;
    k_tree_root[node->idx].depth = node->depth;
    k_tree_root[node->idx].parent_idx = node->parent_idx;
    k_tree_root[node->idx].num_volumes = node->num_volumes;
    k_tree_root[node->idx].num_objects = node->num_objects;
    k_tree_root[node->idx].bmin = node->bbox.min;
    k_tree_root[node->idx].bmax = node->bbox.max;

    for (int i = 0; i < 1024; i++) {
      k_tree_root[node->idx].vol_indices[i] = node->vol_indices[i];
      k_tree_root[node->idx].obj_indices[i] = node->obj_indices[i];
    }

    if (node->has_children) {
      for (int i = 0; i < 8; i++) {
        k_tree_root[node->idx].child_idx[i] = node->child_idx[i];
      }
    }
  }

  dscene->octree_nodes.copy_to_device();
#endif
  need_update = false;
}

void VolumeManager::device_free(Device *device, DeviceScene *dscene)
{
#ifdef WITH_OCTREE
  dscene->octree_nodes.free();
#endif
}

CCL_NAMESPACE_END