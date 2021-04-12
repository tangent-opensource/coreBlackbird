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

#ifndef __OCTREE_NODE_H__
#define __OCTREE_NODE_H__

#include "util/util_boundbox.h"
#include "util/util_defines.h"

CCL_NAMESPACE_BEGIN

/* Octree structure for volumes */

struct ccl_align(16) OCTNode
{
  int num_volumes = 0;
  int num_objects = 0;
  int vol_indices[1024];
  int obj_indices[1024];

  float3 max_extinction = make_float3(0.0f);
  float3 min_extinction = make_float3(FLT_MAX);

  int depth = -1;
  bool has_children = false;

  OCTNode *children[8];
  OCTNode *parent;
  BoundBox bbox;

  /* For flattening the tree */
  int idx = 0;
  int parent_idx = 0;
  int child_idx[8];
};

CCL_NAMESPACE_END

#endif /* __OCTREE_NODE_H__ */
