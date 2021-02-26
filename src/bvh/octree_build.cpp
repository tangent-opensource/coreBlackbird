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

void OCTBuild::update_octree(const vector<Image *> &images)
{
  for (int slot = 0; slot < images.size(); slot++) {

    Image *image = images[slot];

    if (image->metadata.depth > 1) {
      octree_root->bbox.grow(image->metadata.world_bound);

      octree_root->num_volumes++;
      octree_root->vol_indices[slot] = slot;
      octree_root->min_extinction = ccl::min(octree_root->min_extinction, image->metadata.min);
      octree_root->max_extinction = ccl::max(octree_root->max_extinction, image->metadata.max);
    }
  }

  update_root_rec(octree_root, images);
}

void OCTBuild::reset_octree()
{
  clear_root_rec(octree_root);
}

void OCTBuild::build_root_rec(OCTNode *root, int depth)
{
  if (depth > 0) {

    root->bbox = BoundBox(BoundBox::empty);
    memset(root->vol_indices, 0, sizeof(int) * 1024);
    root->depth = depth;

    for (int i = 0; i < 8; i++) {
      root->children[i] = new OCTNode;
      root->children[i]->parent = root;
      root->children[i]->depth = depth;

      build_root_rec(root->children[i], depth - 1);
    }

    root->has_children = true;
  }
}

void OCTBuild::update_root_rec(OCTNode *node, const vector<Image *> &images)
{
  if (node->has_children) {
    for (int i = 0; i < 8; i++) {
      node->children[i]->bbox = divide_bbox(node->bbox, i);
      int vol_idx = 0;
      for (int slot = 0; slot < images.size(); slot++) {

        Image *image = images[slot];

        if (image->metadata.depth > 1) {
          if (image->metadata.world_bound.intersects(node->children[i]->bbox)) {

            node->children[i]->num_volumes++;
            node->children[i]->vol_indices[vol_idx] = slot;
            node->children[i]->min_extinction = ccl::min(node->children[i]->min_extinction,
                                                         image->metadata.min);
            node->children[i]->max_extinction = ccl::max(node->children[i]->max_extinction,
                                                         image->metadata.max);
            vol_idx++;
          }
        }
      }

      update_root_rec(node->children[i], images);
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
  node->has_children = false;
  node->num_volumes = 0;
  node->bbox = BoundBox(BoundBox::empty);
}

CCL_NAMESPACE_END
