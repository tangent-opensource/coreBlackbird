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

void OCTBuild::init_octree()
{
  octree_root = new OCTNode;
  build_root_rec(octree_root, depth);
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

  octree_root->has_children = handles.size() > 0;

  update_root_rec(octree_root, handles);
}

void OCTBuild::reset_octree()
{
  clear_root_rec(octree_root);
}

void OCTBuild::build_root_rec(OCTNode *root, int depth)
{
  root->bbox = BoundBox(BoundBox::empty);
  memset(root->vol_indices, 0, sizeof(int) * 1024);
  root->depth = depth;

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

void OCTBuild::update_root_rec(OCTNode *node, const vector<ImageHandle *> &handles)
{
  if (node->has_children) {
    for (int i = 0; i < 8; i++) {

      node->children[i]->bbox = divide_bbox(node->bbox, i);
      
      int vol_idx = 0;
      for (int i = 0; i < handles.size(); i++) {

        ImageHandle *handle = handles[i];
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

CCL_NAMESPACE_END
