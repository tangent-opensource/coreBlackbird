/*
 * Copyright 2011-2016 Blender Foundation
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
 */

#ifndef __SUBD_PATCH_TABLE_H__
#define __SUBD_PATCH_TABLE_H__

#include "util/util_array.h"
#include "util/util_types.h"

CCL_NAMESPACE_BEGIN

#define PATCH_ARRAY_SIZE 4
#define PATCH_PARAM_SIZE 2
#define PATCH_HANDLE_SIZE 3
#define PATCH_NODE_SIZE 1

struct PackedPatchTable {
  array<uint> table;

  size_t num_arrays;
  size_t num_indices;
  size_t num_patches;
  size_t num_nodes;

  /* calculated size from num_* members */
  size_t total_size() const;
  void copy_adjusting_offsets(uint *dest, int doffset) const;
};

CCL_NAMESPACE_END

#endif /* __SUBD_PATCH_TABLE_H__ */
