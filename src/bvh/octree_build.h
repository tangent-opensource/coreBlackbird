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

#include "util/util_vector.h"

CCL_NAMESPACE_BEGIN

class Progress;
class Image;

/* Octree Builder */

class OCTBuild {
 public:
  /* Constructor/Destructor */
  OCTBuild();
  ~OCTBuild();

  void init_octree();
  void update_octree(vector<Image *> images);
  void reset_octree();

  OCTNode *get_root();
  int get_depth();

 protected:
  /* Internal recursive functions to initialize, update 
  *  and clear the octree structure */
  void build_root_rec(OCTNode *root, int depth);
  void update_root_rec(OCTNode *root, vector<Image *> images);
  void clear_root_rec(OCTNode *root);

  OCTNode *octree_root;

  /* Octree structure params*/
  int depth;
};

CCL_NAMESPACE_END

#endif /* __OCTREE_BUILD_H__ */
