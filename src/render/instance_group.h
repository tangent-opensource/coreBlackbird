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

#include "render/geometry.h"

/* Set of attributes that can be indexed by individual objects 
 * through Object::instance_index. */
class InstanceGroup {
public:
  // todo(Edoardo): Implement subdivision surfaces once they are internal
  InstanceGroup(Geometry* geom, size_t instances) : attributes(geom, ATTR_PRIM_GEOMETRY, instances), attr_map_offset(~0) { }

  AttributeSet attributes;

  /* Used internally by the geometry manager to store the index of
   * the attribute map for this instance group in the device buffer */
  size_t attr_map_offset;
};

CCL_NAMESPACE_END

#endif /* __INSTANCE_GROUP_H__ */