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

/*
 * SDF Functions based on:
 *
 * - https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
 * - http://mercury.sexy/hg_sdf/
 */

ccl_device_inline float ndot(float2 a, float2 b)
{
  return a.x * b.x - a.y * b.y;
}

ccl_device_inline float sdf_2d_circle(float2 p)
{
  return len(p) - 1.0f;
}

ccl_device_inline float sdf_2d_line(float2 p, float2 a, float2 b)
{
  float2 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return len(pa - ba * h);
}

ccl_device_inline float sdf_2d_box(float2 p, float2 b)
{
  float2 d = fabs(p) - b;
  return len(max(d, make_float2(0.0f, 0.0f))) + min(max(d.x, d.y), 0.0f);
}

ccl_device_inline float sdf_2d_rhombus(float2 p, float2 b)
{
  float2 q = fabs(p);
  float h = clamp((-2.0f * ndot(q, b) + ndot(b, b)) / dot(b, b), -1.0f, 1.0f);
  float d = len(q - 0.5f * b * make_float2(1.0f - h, 1.0f + h));
  return d * signf(q.x * b.y + q.y * b.x - b.x * b.y);
}

ccl_device_inline float sdf_2d_hexagon(float2 p, float r)
{
  const float3 k = make_float3(-0.866025404f, 0.5f, 0.577350269f);
  float2 kxy = make_float2(k.x, k.y);
  p = fabs(p);
  p = p - (2.0f * min(dot(kxy, p), 0.0f) * kxy);
  p = p - make_float2(clamp(p.x, -k.z * r, k.z * r), r);
  return len(p) * signf(p.y);
}

ccl_device_inline float sdf_2d_triangle(float2 p, float r)
{
  const float k = sqrt(3.0f);
  p.x = fabsf(p.x) - r;
  p.y = p.y + r / k;
  if (p.x + k * p.y > 0.0f)
    p = make_float2(p.x - k * p.y, -k * p.x - p.y) / 2.0f;
  p.x -= clamp(p.x, -2.0f * r, 0.0f);
  return -len(p) * signf(p.y);
}

ccl_device_inline float sdf_2d_star(float2 p, float r, int n, float m)
{
  float an = M_PI_F / float(n);
  float en = M_PI_F / m;  // m is between 2 and n
  float2 acs = make_float2(cos(an), sin(an));
  float2 ecs = make_float2(cos(en), sin(en));  // ecs=make_float2(0,1) for regular polygon,

  float bn = compatible_mod(atan2f(p.x, p.y), 2.0f * an) - an;
  p = len(p) * make_float2(cos(bn), fabsf(sin(bn)));
  p = p - r * acs;
  p += ecs * clamp(-dot(p, ecs), 0.0f, safe_divide(r * acs.y, ecs.y));
  return len(p) * signf(p.x);
}

ccl_device_inline float sdf_3d_hex_prism(float3 p, float3 s)
{
  float3 q = fabs(p);
  return max(q.z - s.y, max((q.x * 0.866025f + q.y * 0.5f), q.y) - s.x);
}

ccl_device_inline float sdf_3d_sphere(float3 p, float s)
{
  return len(p) - s;
}

ccl_device_inline float sdf_3d_box(float3 p, float3 b)
{
  float3 q = fabs(p) - b;
  return len(max(q, make_float3(0.0f))) + min(max(q.x, max(q.y, q.z)), 0.0f);
}

ccl_device_inline float sdf_3d_torus(float3 p, float2 t)
{
  float2 pxz = make_float2(p.x, p.z);
  float2 q = make_float2(len(pxz) - t.x, p.y);
  return len(q) - t.y;
}

ccl_device_inline float sdf_3d_cone(float3 p, float a)
{
  // c is the sin/cos of the angle
  float2 c = make_float2(sin(a), cos(a));
  float2 pxy = make_float2(p.x, p.y);
  float q = len(pxy);
  return dot(c, make_float2(q, p.z));
}

ccl_device_inline float sdf_3d_cylinder(float3 p, float3 c)
{
  float2 pxz = make_float2(p.x, p.z);
  float2 cxy = make_float2(c.x, c.y);
  return len(pxz - cxy) - c.z;
}

ccl_device_inline float sdf_3d_capsule(float3 p, float3 a, float3 b)
{
  float3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
  return len(pa - ba * h);
}

ccl_device_inline float sdf_3d_octahedron(float3 p)
{
  p = fabs(p);
  float m = p.x + p.y + p.z - 1.0f;
  float3 q;
  if (3.0f * p.x < m) {
    q = make_float3(p.x, p.y, p.z);
  }
  else if (3.0f * p.y < m) {
    q = make_float3(p.y, p.z, p.x);
  }
  else if (3.0f * p.z < m) {
    q = make_float3(p.z, p.x, p.y);
  }
  else {
    return m * 0.57735027f;
  }

  float k = clamp(0.5f * (q.z - q.y + 1.0f), 0.0f, 1.0f);
  return len(make_float3(q.x, q.y - 1.0f + k, q.z - k));
}

/* Utility functions. */

ccl_device float sdf_alteration(
    float fac, float size, float rounding, float thickness, bool invert)
{
  // fac -= (rounding / size);

  if (thickness != 0.0) {
    fac = fabsf(fac) - thickness;
  }

  fac = (invert) ? -fac : fac;

  return fac;
}

/* Sdf Ops */

ccl_device_inline float sdf_op_union(float a, float b)
{
  return min(a, b);
}

ccl_device_inline float sdf_op_intersection(float a, float b)
{
  return max(a, b);
}

ccl_device_inline float sdf_op_difference(float a, float b)
{
  return max(-a, b);
}

ccl_device_inline float sdf_op_union_smooth(float a, float b, float k)
{
  float h = safe_divide(max(k - fabsf(a - b), 0.0f), k);
  return min(a, b) - h * h * h * k * (1.0f / 6.0f);
}

ccl_device_inline float sdf_op_difference_smooth(float a, float b, float k)
{
  float h = clamp(0.5f - 0.5f * (b + a) / k, 0.0f, 1.0f);
  return mix(b, -a, h) + k * h * (1.0f - h);
}

ccl_device_inline float sdf_op_intersection_smooth(float a, float b, float k)
{
  float h = clamp(safe_divide(0.5f - 0.5f * (b - a), k), 0.0f, 1.0f);
  return mix(b, a, h) + k * h * (1.0f - h);
}

ccl_device_inline float sdf_op_round(float a, float k)
{
  return a - k;
}

ccl_device_inline float sdf_op_onion(float a, float k, int n)
{
  if (n > 0) {
    float onion = fabsf(a) - k;
    for (int i = 0; i < n; i++) {
      k *= 0.5f;
      onion = fabsf(onion) - k;
    }
    return onion;
  }
  else {
    return fabsf(a) - k;
  }
}

ccl_device_inline float sdf_op_blend(float a, float b, float k)
{
  return mix(a, b, k);
}

CCL_NAMESPACE_END
