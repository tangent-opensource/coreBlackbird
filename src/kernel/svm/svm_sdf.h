/*
 * Copyright 2011-2020 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
ee the License for the specific language governing permissions and
 * limitations under the License.
 */

CCL_NAMESPACE_BEGIN

/* Sdf Shader Functions */
ccl_device float svm_sdf(float3 vec,
                         float size,
                         float thickness,
                         int sides,
                         float v1,
                         float v2,
                         float v3,
                         float v4,
                         float3 p1,
                         float3 p2,
                         float3 p3,
                         float a1,
                         float a2,
                         bool invert,
                         NodeSdfMode mode)
{
  float fac = 0.0f;
  float s = size;

  if (s != 0.0f) {

    switch (mode) {
      case NODE_SDF_2D_PIE:
      case NODE_SDF_2D_ARC:
      case NODE_SDF_2D_BEZIER:
      case NODE_SDF_2D_UNEVEN_CAPSULE:
      case NODE_SDF_2D_POINT_TRIANGLE:
      case NODE_SDF_2D_TRAPEZOID:
      case NODE_SDF_2D_VESICA:
      case NODE_SDF_2D_CROSS:
      case NODE_SDF_2D_ROUNDX:
      case NODE_SDF_2D_HORSESHOE:
      case NODE_SDF_2D_PARABOLA:
      case NODE_SDF_2D_ELLIPSE:
      case NODE_SDF_2D_ISOSCELES:
      case NODE_SDF_2D_ROUND_JOINT:

        break;
      case NODE_SDF_3D_SPHERE:
        fac = sdf_3d_sphere(vec, v1);
        break;
      case NODE_SDF_3D_HEX_PRISM:
        fac = sdf_3d_hex_prism(vec, make_float3(v1, v2, v3));
        break;
      case NODE_SDF_3D_BOX:
        fac = sdf_3d_box(vec, make_float3(v1, v2, v3));
        break;
      case NODE_SDF_3D_TORUS:
        fac = sdf_3d_torus(vec, make_float2(v1, v2));
        break;
      case NODE_SDF_3D_CONE:
        fac = sdf_3d_cone(vec, a1);
      case NODE_SDF_3D_CYLINDER:
        fac = sdf_3d_cylinder(vec, make_float3(v1, v2, v3));
        break;
      case NODE_SDF_3D_CAPSULE:
        fac = sdf_3d_capsule(vec, p1, p2);
        break;
      case NODE_SDF_3D_OCTAHEDRON:
        fac = sdf_3d_octahedron(vec);
        break;
      case NODE_SDF_2D_CIRCLE:
        fac = sdf_2d_circle(float3_to_float2(vec));
        break;
      case NODE_SDF_2D_BOX:
        fac = sdf_2d_box(float3_to_float2(vec), make_float2(v1, v2));
        break;
      case NODE_SDF_2D_RHOMBUS:
        fac = sdf_2d_rhombus(float3_to_float2(vec), make_float2(v1, v2));
        break;
      case NODE_SDF_2D_TRIANGLE:
        fac = sdf_2d_triangle(float3_to_float2(vec), v1);
        break;
      case NODE_SDF_2D_LINE:
        fac = sdf_2d_line(float3_to_float2(vec), float3_to_float2(p1), float3_to_float2(p2));
        break;
      case NODE_SDF_2D_STAR:
        fac = sdf_2d_star(float3_to_float2(vec), v1, sides, v2);
        break;
      case NODE_SDF_2D_HEXAGON:
        fac = sdf_2d_hexagon(float3_to_float2(vec), v1);
        break;
    }
    fac = sdf_alteration(fac, size, v4, thickness, invert);
  }
  return fac;
}

ccl_device float svm_sdf_op(float a, float b, float r, float r2, float n, NodeSdfOps type)
{
  float fac = 0.0f;
  switch (type) {
    case NODE_SDF_OP_ROUND:
      fac = sdf_op_round(a, r);
      break;
    case NODE_SDF_OP_ONION:
      fac = sdf_op_onion(a, r, (int)n);
      break;
    case NODE_SDF_OP_BLEND:
      fac = sdf_op_blend(a, b, r);
      break;
    case NODE_SDF_OP_UNION:
      fac = sdf_op_union(a, b);
      break;
    case NODE_SDF_OP_INTERSECTION:
      fac = sdf_op_intersection(a, b);
      break;
    case NODE_SDF_OP_DIFFERENCE:
      fac = sdf_op_difference(a, b);
      break;
    case NODE_SDF_OP_UNION_SMOOTH:
      fac = sdf_op_union_smooth(a, b, r);
      break;
    case NODE_SDF_OP_INTERSECTION_SMOOTH:
      fac = sdf_op_intersection_smooth(a, b, r);
      break;
    case NODE_SDF_OP_DIFFERENCE_SMOOTH:
      fac = sdf_op_difference_smooth(a, b, r);
      break;
  }
  return fac;
}

/* Nodes */

ccl_device void svm_node_sdf_primitives(
    KernelGlobals *kg, ShaderData *sd, float *stack, uint4 node, int *offset)
{
  uint4 node_inout = read_node(kg, offset);
  uint4 node_points = read_node(kg, offset);
  uint4 node_defaults = read_node(kg, offset);
  uint4 node_defaults2 = read_node(kg, offset);

  /* Input and Output Sockets */
  uint size_stack_offset, thickness_stack_offset, sides_stack_offset;
  uint value1_stack_offset, value2_stack_offset, value3_stack_offset, value4_stack_offset;
  uint mode, invert, angle1_stack_offset, angle2_stack_offset;

  svm_unpack_node_uchar3(node.y,
                         &size_stack_offset,
                         &thickness_stack_offset,
                         &sides_stack_offset);
  svm_unpack_node_uchar4(node.z,
                         &value1_stack_offset,
                         &value2_stack_offset,
                         &value3_stack_offset,
                         &value4_stack_offset);
  svm_unpack_node_uchar4(node.w, &mode, &invert, &angle1_stack_offset, &angle2_stack_offset);

  uint vector_in = node_inout.x;
  uint distance_out = node_inout.y;

  float size = stack_load_float_default(stack, size_stack_offset, node_defaults.x);
  float thickness = stack_load_float_default(stack, thickness_stack_offset, node_defaults.y);
  float a1 = stack_load_float_default(stack, angle1_stack_offset, node_defaults.z);
  float a2 = stack_load_float_default(stack, angle2_stack_offset, node_defaults.w);

  int sides = stack_load_int(stack, sides_stack_offset);

  float v1 = stack_load_float_default(stack, value1_stack_offset, node_defaults2.x);
  float v2 = stack_load_float_default(stack, value2_stack_offset, node_defaults2.y);
  float v3 = stack_load_float_default(stack, value3_stack_offset, node_defaults2.z);
  float v4 = stack_load_float_default(stack, value4_stack_offset, node_defaults2.w);

  float3 vector = stack_load_float3(stack, vector_in);
  float3 p1 = stack_load_float3(stack, node_points.x);
  float3 p2 = stack_load_float3(stack, node_points.y);
  float3 p3 = stack_load_float3(stack, node_points.z);

  if (stack_valid(distance_out)) {
    float distance = svm_sdf(vector,
                             size,
                             thickness,
                             sides,
                             v1,
                             v2,
                             v3,
                             v4,
                             p1,
                             p2,
                             p3,
                             a1,
                             a2,
                             (bool)invert,
                             (NodeSdfMode)mode);
    stack_store_float(stack, distance_out, distance);
  }
}

ccl_device void svm_node_sdf_ops(
    KernelGlobals *kg, ShaderData *sd, float *stack, uint4 node, int *offset)
{
  uint4 node1 = read_node(kg, offset);
  uint4 node2 = read_node(kg, offset);

  /* Input and Output Sockets */
  uint value1_stack_offset, value2_stack_offset, radius1_stack_offset;
  uint radius2_stack_offset, count_stack_offset, distance_stack_offset;

  svm_unpack_node_uchar4(node.z,
                         &value1_stack_offset,
                         &value2_stack_offset,
                         &radius1_stack_offset,
                         &radius2_stack_offset);
  svm_unpack_node_uchar2(node.w, &count_stack_offset, &distance_stack_offset);

  float value1 = stack_load_float_default(stack, value1_stack_offset, node1.x);
  float value2 = stack_load_float_default(stack, value2_stack_offset, node1.y);
  float radius1 = stack_load_float_default(stack, radius1_stack_offset, node1.z);
  float radius2 = stack_load_float_default(stack, radius2_stack_offset, node1.w);
  float count = stack_load_float_default(stack, count_stack_offset, node2.x);

  if (stack_valid(distance_stack_offset)) {
    float dist = svm_sdf_op(value1, value2, radius1, radius2, count, (NodeSdfOps)node.y);
    stack_store_float(stack, distance_stack_offset, dist);
  }
}

CCL_NAMESPACE_END
