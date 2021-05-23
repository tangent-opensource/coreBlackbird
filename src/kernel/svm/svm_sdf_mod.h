/*
 * Copyright 2011-2020 Blender Foundation
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

CCL_NAMESPACE_BEGIN

/* Vector Rotate */

ccl_device void svm_node_sdf_mod(ShaderData *sd,
                                       float *stack,
                                       uint input_stack_offsets,
                                       uint axis_stack_offsets,
                                       uint result_stack_offset)
{
  uint type, vector_stack_offset, rotation_stack_offset, center_stack_offset, axis_stack_offset,
      angle_stack_offset;

  svm_unpack_node_uchar3(input_stack_offsets, &type, &vector_stack_offset, &rotation_stack_offset);
  svm_unpack_node_uchar3(
      axis_stack_offsets, &center_stack_offset, &axis_stack_offset, &angle_stack_offset);

  float3 vector = stack_load_float3(stack, vector_stack_offset);
  float3 center = stack_load_float3(stack, center_stack_offset);
  float3 result = make_float3(0.0f, 0.0f, 0.0f);

    switch (type) {
    default:
        break;
    }

  
  /* Output */
  if (stack_valid(result_stack_offset)) {
    stack_store_float3(stack, result_stack_offset, result);
  }
}

CCL_NAMESPACE_END
