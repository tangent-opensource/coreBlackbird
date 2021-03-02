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

  octree_builder = new OCTBuild;
  octree_builder->init_octree();
}

VolumeManager::~VolumeManager()
{
  octree_builder->reset_octree();
}

void VolumeManager::device_update(Device *device, Scene *scene, Progress &progress)
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

  octree_builder->reset_octree();

  /* Parallel update volume object world bounding boxes */
  static const int OBJECTS_PER_TASK = 32;
  parallel_for(blocked_range<size_t>(0, scene->objects.size(), OBJECTS_PER_TASK),
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

  vector<ImageHandle *> image_handles;

  foreach (Object *object, scene->objects) {
    foreach (Attribute &attr, object->geometry->attributes.attributes) {
      if (attr.element == ATTR_ELEMENT_VOXEL) {
        ImageHandle &handle = attr.data_voxel();
        image_handles.push_back(&handle);
      }
    }
  }

  octree_builder->update_octree(image_handles);

  need_update = false;
}

void VolumeManager::device_free(Device *device, DeviceScene *dscene)
{
}

CCL_NAMESPACE_END