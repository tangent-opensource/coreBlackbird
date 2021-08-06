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
 *
 * Based on Embree code, copyright 2009-2020 Intel Corporation.
 */

CCL_NAMESPACE_BEGIN

/* Point primitive intersection functions. */

#ifdef __POINTCLOUD__

ccl_device_forceinline bool point_intersect_test_sphere(const float4 point,
                                                        const float3 P,
                                                        const float3 dir,
                                                        float *t)
{
  const float3 center = float4_to_float3(point);
  const float radius = point.w;

  const float rd2 = 1.0f / dot(dir, dir);

  const float3 c0 = center - P;
  const float projC0 = dot(c0, dir) * rd2;
  const float3 perp = c0 - projC0 * dir;
  const float l2 = dot(perp, perp);
  const float r2 = radius * radius;
  if (!(l2 <= r2)) {
    return false;
  }

  const float td = sqrt((r2 - l2) * rd2);
  const float t_front = projC0 - td;
  const float t_back = projC0 + td;

  const bool valid_front = (0.0f <= t_front) & (t_front <= *t);
  const bool valid_back = (0.0f <= t_back) & (t_back <= *t);

  /* check if there is a first hit */
  const bool valid_first = valid_front | valid_back;
  if (!valid_first) {
    return false;
  }

  *t = (valid_front) ? t_front : t_back;
  return true;
}

ccl_device_forceinline bool point_intersect_test_disc(const float4 point,
                                                      const float3 P,
                                                      const float3 dir,
                                                      float *t)
{

  /* Would a min radius help with stability? */
  const float3 center = float4_to_float3(point);
  const float radius = point.w;

  const float d2 = dot(dir, dir);
  const float rd2 = 1.f / d2;
  const float rd = 1.f / sqrt(d2);

  const float3 c0 = center - P;
  const float projC0 = dot(c0, dir) * rd2;

  if (*t <= projC0 || projC0 < 0.f) {
    return false;
  }

  /* Self-intersection */
  const float avoidance_factor = 2.0f * radius * rd;
  if (projC0 < avoidance_factor) {
    return false;
  }

  if (projC0 < radius) {
    return false;
  }

  const float3 perp = c0 - projC0 * dir;
  const float l2 = dot(perp, perp);
  const float r2 = radius * radius;
  if (!(l2 <= r2)) {
    return false;
  }

  *t = projC0;
  return true;
}

ccl_device_forceinline bool point_intersect_test_disc_oriented(
    const float4 point, const float3 P, const float3 N, const float3 dir, float *t)
{

  const float3 center = float4_to_float3(point);
  const float radius = point.w;

  const float divisor = dot(dir, N);
  if (divisor == 0.f) {  // parallel
    return false;
  }

  const float t_proj = dot(center - P, N) / divisor;

  if (*t <= t_proj || t_proj < 0.f) {
    return false;
  }

  const float3 intersection = P + dir * t_proj;
  const float dist2 = dot(intersection - center, intersection - center);
  if (dist2 > (radius * radius)) {
    return false;
  }

  *t = t_proj;

  return true;
}

ccl_device_forceinline bool point_intersect(KernelGlobals *kg,
                                            Intersection *isect,
                                            float3 P,
                                            float3 dir,
                                            uint visibility,
                                            int object,
                                            int prim_addr,
                                            float time,
                                            int type)
{
  const bool is_motion = (type & PRIMITIVE_ALL_MOTION);

#  ifndef __KERNEL_OPTIX__ /* See OptiX motion flag OPTIX_MOTION_FLAG_[START|END]_VANISH */
  if (is_motion && kernel_data.bvh.use_bvh_steps) {
    const float2 prim_time = kernel_tex_fetch(__prim_time, prim_addr);
    if (time < prim_time.x || time > prim_time.y) {
      return false;
    }
  }
#  endif

  const int prim = kernel_tex_fetch(__prim_index, prim_addr);

  float4 point;
  const int n_attrs = (type == PRIMITIVE_POINT_DISC_ORIENTED ||
                       type == PRIMITIVE_MOTION_POINT_DISC_ORIENTED) ?
                          2 :
                          1;

  const int fobject = (object == OBJECT_NONE) ? kernel_tex_fetch(__prim_object, prim_addr) :
                                                object;
  if (!is_motion) {
    point = kernel_tex_fetch(__points, prim * n_attrs);
  }
  else {
    point = motion_point_attribute(
        kg, fobject, prim, time, n_attrs, ATTR_STD_MOTION_VERTEX_POSITION, 0);
  }

  float t = isect->t;

  bool ret_test;
  if (type == PRIMITIVE_POINT_SPHERE || type == PRIMITIVE_MOTION_POINT_SPHERE) {
    ret_test = point_intersect_test_sphere(point, P, dir, &t);
  }
  else if (type == PRIMITIVE_POINT_DISC || type == PRIMITIVE_MOTION_POINT_DISC) {
    ret_test = point_intersect_test_disc(point, P, dir, &t);
  }
  else if (type == PRIMITIVE_POINT_DISC_ORIENTED || type == PRIMITIVE_MOTION_POINT_DISC_ORIENTED) {
    float3 normal;
    if (!is_motion) {
      normal = float4_to_float3(kernel_tex_fetch(__points, prim * n_attrs + 1));
    }
    else {
      normal = float4_to_float3(motion_point_attribute(
          kg, fobject, prim, time, n_attrs, ATTR_STD_MOTION_VERTEX_NORMAL, 1));
    }

    ret_test = point_intersect_test_disc_oriented(point, P, normal, dir, &t);
  }
  if (!ret_test) {
    return false;
  }

#  ifdef __VISIBILITY_FLAG__
  /* Visibility flag test. we do it here under the assumption
   * that most points are culled by node flags.
   */
  if (!(kernel_tex_fetch(__prim_visibility, prim_addr) & visibility)) {
    return false;
  }
#  endif

  isect->prim = prim_addr;
  isect->object = object;
  isect->type = type;
  isect->u = 0.0f;
  isect->v = 0.0f;
  isect->t = t;
  return true;
}

ccl_device_inline void point_shader_setup(KernelGlobals *kg,
                                          ShaderData *sd,
                                          const Intersection *isect,
                                          const Ray *ray)
{
  const bool is_motion = isect->type & PRIMITIVE_ALL_MOTION;

  sd->shader = kernel_tex_fetch(__points_shader, sd->prim);
  sd->P = ray->P + ray->D * isect->t;

  /* Texture coordinates, zero for now. */
#  ifdef __UV__
  sd->u = isect->u;
  sd->v = isect->v;
#  endif

  /* Computer point center for normal. */
  const int n_attrs = (sd->type == PRIMITIVE_POINT_DISC_ORIENTED ||
                       sd->type == PRIMITIVE_MOTION_POINT_DISC_ORIENTED) ?
                          2 :
                          1;
  float3 center;
  if (!is_motion) {
    center = float4_to_float3(kernel_tex_fetch(__points, sd->prim * n_attrs));
  }
  else {
    center = float4_to_float3(motion_point_attribute(
        kg, sd->object, sd->prim, sd->time, n_attrs, ATTR_STD_MOTION_VERTEX_POSITION, 0));
  }

  if (isect->object != OBJECT_NONE) {
#  ifdef __OBJECT_MOTION__
    Transform tfm = sd->ob_tfm;
#  else
    Transform tfm = object_fetch_transform(kg, isect->object, OBJECT_TRANSFORM);
#  endif

    center = transform_point(&tfm, center);
  }

  /* Normal */
  if (sd->type == PRIMITIVE_POINT_SPHERE || sd->type == PRIMITIVE_MOTION_POINT_SPHERE) {
    sd->Ng = normalize(sd->P - center);
  }
  else if (sd->type == PRIMITIVE_POINT_DISC || sd->type == PRIMITIVE_MOTION_POINT_DISC) {
    sd->Ng = normalize(-ray->D);
  }
  else if (sd->type == PRIMITIVE_POINT_DISC_ORIENTED ||
           sd->type == PRIMITIVE_MOTION_POINT_DISC_ORIENTED) {
    /* todo: This buffer should be obtained from Embree */
    if (!is_motion) {
      sd->Ng = float4_to_float3(kernel_tex_fetch(__points, sd->prim * n_attrs + 1));
    }
    else {
      sd->Ng = float4_to_float3(motion_point_attribute(
          kg, sd->object, sd->prim, sd->time, n_attrs, ATTR_STD_MOTION_VERTEX_NORMAL, 1));
    }
  }
  if (isect->object != OBJECT_NONE) {
    object_inverse_normal_transform(kg, sd, &sd->Ng);
  }
  sd->N = sd->Ng;

#  ifdef __DPDU__
  /* dPdu/dPdv */
  sd->dPdu = make_float3(0.0f, 0.0f, 0.0f);
  sd->dPdv = make_float3(0.0f, 0.0f, 0.0f);
#  endif
}

#endif

CCL_NAMESPACE_END
