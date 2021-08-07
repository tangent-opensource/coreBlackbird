/*
 * Based on code from OpenSubdiv released under this license:
 *
 * Copyright 2014 DreamWorks Animation LLC.
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification to it:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *   names, trademarks, service marks, or product names of the Licensor
 *   and its affiliates, except as required to comply with Section 4(c) of
 *   the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the Apache License for the specific
 * language governing permissions and limitations under the Apache License.
 */

#include "subd/subd_patch_table.h"
#include "kernel/kernel_types.h"

#include "util/util_math.h"

CCL_NAMESPACE_BEGIN

/* packed patch table functions */

size_t PackedPatchTable::total_size() const
{
  return num_arrays * PATCH_ARRAY_SIZE + num_indices +
         num_patches * (PATCH_PARAM_SIZE + PATCH_HANDLE_SIZE) + num_nodes * PATCH_NODE_SIZE;
}

void PackedPatchTable::copy_adjusting_offsets(uint *dest, int doffset) const
{
  const uint *src = table.data();

  /* arrays */
  for (int i = 0; i < num_arrays; i++) {
    *(dest++) = *(src++);
    *(dest++) = *(src++);
    *(dest++) = *(src++) + doffset;
    *(dest++) = *(src++) + doffset;
  }

  /* indices */
  for (int i = 0; i < num_indices; i++) {
    *(dest++) = *(src++);
  }

  /* params */
  for (int i = 0; i < num_patches; i++) {
    *(dest++) = *(src++);
    *(dest++) = *(src++);
  }

  /* handles */
  for (int i = 0; i < num_patches; i++) {
    *(dest++) = *(src++) + doffset;
    *(dest++) = *(src++) + doffset;
    *(dest++) = *(src++);
  }

  /* nodes */
  for (int i = 0; i < num_nodes; i++) {
    *(dest++) = *(src++) + doffset;
  }
}

CCL_NAMESPACE_END
