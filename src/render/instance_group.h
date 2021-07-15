/*
*  Copyright 2021 Tangent Animation
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
*  including without limitation, as related to merchantability and fitness
*  for a particular purpose.
*
*  In no event shall any copyright holder be liable for any damages of any kind
*  arising from the use of this software, whether in contract, tort or otherwise.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
#ifndef __INSTANCE_GROUP_H__
#define __INSTANCE_GROUP_H__

CCL_NAMESPACE_BEGIN

// todo: forward declaration
#include "geometry.h"

/* Stores a set of attributes that can be indexed by individual objects 
 * through Object::instance_index.
 *
 * Note: All objects that reference a certain instance group need to have
 * the same geometry as stored in the instance group.
 * 
 * There is no InstanceManager, because the logic to upload attribute
 * and respective maps is interwined with the geometry manager.
 * */
struct InstanceGroup {
  // todo(Edo): An extra parameter might be needed once we have internal subdivision surfaces
  InstanceGroup(Geometry* geom) : attributes(geom, ATTR_PRIM_GEOMETRY), geometry(geom) { }

  AttributeSet attributes;
  Geometry* geometry = nullptr;

  /* Used by the geometry manager */
  size_t attr_map_offset = 0;
};

CCL_NAMESPACE_END

#endif /* __INSTANCE_GROUP_H__ */