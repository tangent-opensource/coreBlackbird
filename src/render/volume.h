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

#ifndef __VOLUME_H__
#define __VOLUME_H__

#include "util/util_vector.h"

CCL_NAMESPACE_BEGIN

class Device;
class DeviceScene;
class Scene;
class Progress;
class Object;
class OCTBuild;

/* Volume Manager */

class VolumeManager {
 public:
  bool need_update;

  VolumeManager();
  ~VolumeManager();

  void device_update(DeviceScene *dscene, Scene *scene, Progress &progress);

  void device_update_octree(DeviceScene *dscene, Scene *scene, Progress &progress);

  void device_free(Device *device, DeviceScene *dscene);

 private:
  OCTBuild *octree_builder;
  vector<Object *> volume_objects;
};

CCL_NAMESPACE_END

#endif  // !__VOLUME_H__
