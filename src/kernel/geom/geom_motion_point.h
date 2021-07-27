/*
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

/* Motion Point Primitive
 *
 * These are stored as regular points, plus extra positions and radii at times
 * other than the frame center. Computing the point at a given ray time is
 * a matter of interpolation of the two steps between which the ray time lies.
 *
 * The extra points are stored as ATTR_STD_MOTION_VERTEX_POSITION.
 */

#ifdef __POINTCLOUD__

ccl_device_inline int find_attribute_point_motion(KernelGlobals *kg,
                                                  int object,
                                                  uint id,
                                                  AttributeElement *elem)
{
  uint attr_offset = object_attribute_map_offset(kg, object) + ATTR_PRIM_GEOMETRY;
  uint4 attr_map = kernel_tex_fetch(__attributes_map, attr_offset);

  while (attr_map.x != id) {
    attr_offset += ATTR_PRIM_TYPES;
    attr_map = kernel_tex_fetch(__attributes_map, attr_offset);
  }

  *elem = (AttributeElement)attr_map.y;

  /* return result */
  return (attr_map.y == ATTR_ELEMENT_NONE) ? (int)ATTR_STD_NOT_FOUND : (int)attr_map.z;
}

ccl_device_inline float4 motion_point_attribute_for_step(KernelGlobals *kg,
                                                         int offset,
                                                         int numverts,
                                                         int numsteps,
                                                         int step,
                                                         int prim,
                                                         int n_points_attrs,
                                                         int attr)
{
  if (step == numsteps) {
    /* center step: regular key location */
    return kernel_tex_fetch(__points, prim * n_points_attrs + attr);
  }
  else {
    /* center step is not stored in this array */
    if (step > numsteps)
      step--;

    offset += step * numverts;

    return kernel_tex_fetch(__attributes_float3, offset + prim);
  }
}

/* return 2 point key locations */
ccl_device_inline float4 motion_point_attribute(KernelGlobals *kg,
                                                int object,
                                                int prim,
                                                float time,
                                                int n_points_attrs,
                                                uint id_attr,
                                                int attr)
{
  /* get motion info */
  int numsteps, numverts;
  object_motion_info(kg, object, &numsteps, &numverts, NULL);

  /* figure out which steps we need to fetch and their interpolation factor */
  int maxstep = numsteps * 2;
  int step = min((int)(time * maxstep), maxstep - 1);
  float t = time * maxstep - step;

  /* find attribute */
  AttributeElement elem;
  int offset = find_attribute_point_motion(kg, object, id_attr, &elem);
  kernel_assert(offset != ATTR_STD_NOT_FOUND);

  /* fetch key coordinates */
  float4 point_attr = motion_point_attribute_for_step(
      kg, offset, numverts, numsteps, step, prim, n_points_attrs, attr);
  float4 next_point_attr = motion_point_attribute_for_step(
      kg, offset, numverts, numsteps, step + 1, prim, n_points_attrs, attr);

  /* interpolate between steps */
  return (1.0f - t) * point_attr + t * next_point_attr;
}

#endif

CCL_NAMESPACE_END
