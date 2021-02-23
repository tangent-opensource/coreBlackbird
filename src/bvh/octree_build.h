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

#ifndef __OCTREE_BUILD_H__
#define __OCTREE_BUILD_H__

#include "bvh/octree_node.h"

#include "util/util_progress.h"
#include "util/util_task.h"
#include "util/util_vector.h"

CCL_NAMESPACE_BEGIN

class Mesh;
class Progress;
class Object;

/* Octree Builder */

class OCTBuild {
 public:
  /* Constructor/Destructor */
  OCTBuild();
  ~OCTBuild();

  OCTNode *build_root(vector<Object *> &objects);

 protected:
  /* Progress. */
  void progress_update();

  /* Progress reporting. */
  Progress &progress;
  double progress_start_time;
  size_t progress_count;
  size_t progress_total;
  size_t progress_original_total;

}

CCL_NAMESPACE_END

#endif /* __OCTREE_BUILD_H__ */
