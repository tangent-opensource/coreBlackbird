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

#include "render/mesh.h"
#include "render/object.h"

#include "util/util_algorithm.h"
#include "util/util_foreach.h"
#include "util/util_logging.h"
#include "util/util_progress.h"
#include "util/util_queue.h"
#include "util/util_simd.h"
#include "util/util_stack_allocator.h"
#include "util/util_time.h"

CCL_NAMESPACE_BEGIN

/* Constructor / Destructor */
OCTBuild::OCTBuild() : depth(3)
{
}

OCTBuild::~OCTBuild()
{
}

void OCTBuild::init_octree()
{
  octree_root = new OCTNode;
  build_root_rec(octree_root, depth);
}

void OCTBuild::update_octree(vector<Image *> images)
{
  update_root_rec(octree_root, images);
}

void OCTBuild::reset_octree()
{
  clear_root_rec(octree_root);
}

void OCTBuild::build_root_rec(OCTNode *root, int depth)
{
  if (depth > 0) {
    for (int i = 0; i < 8; i++) {
      root->children[i] = new OCTNode;
      root->children[i]->parent = root;
      root->children[i]->depth = depth;

      build_root_rec(root->children[i], depth - 1);
    }

    root->has_children = true;
  }
}

void OCTBuild::update_root_rec(OCTNode *node, vector<Image *> images)
{
  if (node->has_children) {
    for (int i = 0; i < 8; i++) {
      node->children[i]->bbox = divide_bbox(node->bbox, i);
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

  node->max_extinction = 0.0f;
  node->min_extinction = 1e10f;
  node->has_children = false;
  node->depth = -1;
  node->num_volumes = 0;
  node->bbox = BoundBox();
}

CCL_NAMESPACE_END
