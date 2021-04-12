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
 */

/* This is a template octree traversal function for volume aggregate structure */

#ifndef __KERNEL_GPU__
ccl_device
#else
ccl_device_inline
#endif
    bool BVH_FUNCTION_FULL_NAME(BVH)(KernelGlobals *kg,
                                     const Ray *ray,
                                     Intersection *isect,
                                     const uint visibility)
{
  /* ray parameters in registers */
  float3 P = ray->P;
  float3 dir = bvh_clamp_direction(ray->D);
  float3 idir = bvh_inverse_direction(dir);

  isect->prim = PRIM_NONE;
  isect->object = OBJECT_NONE;
  isect->has_volume = false;
  isect->in_volume = false;

  KernelOCTree root = kernel_tex_fetch(__octree_nodes, 0);

  if (root.num_objects == 0) {
    return false;
  }

  float t_min = FLT_MAX, t_max = -FLT_MAX;

  float t1 = (root.bmin.x - P.x) * idir.x;
  float t2 = (root.bmax.x - P.x) * idir.x;
  float t3 = (root.bmin.y - P.y) * idir.y;
  float t4 = (root.bmax.y - P.y) * idir.y;
  float t5 = (root.bmin.z - P.z) * idir.z;
  float t6 = (root.bmax.z - P.z) * idir.z;
  t_min = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
  t_max = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));
  if (t_max <= 0.0f) {
    return false;  // box is behind
  }
  if (t_min > ray->t) {
    return false; // Object in between
  }
  if (t_min > t_max) {
    return false;  // ray missed
  }
  if (t_min < 0) {
    t_min = 0.0f;
  }

  isect->v_t = t_min;
  isect->has_volume = true;
  if (P.x >= root.bmin.x && P.x <= root.bmax.x && 
      P.y >= root.bmin.y && P.y <= root.bmax.y &&
      P.z >= root.bmin.z && P.z <= root.bmax.z) {
    isect->in_volume = true;
  }
  return true;
}

ccl_device_inline bool BVH_FUNCTION_NAME(KernelGlobals *kg,
                                         const Ray *ray,
                                         Intersection *isect,
                                         const uint visibility)
{
  return BVH_FUNCTION_FULL_NAME(BVH)(kg, ray, isect, visibility);
}

#undef BVH_FUNCTION_NAME
#undef BVH_FUNCTION_FEATURES
