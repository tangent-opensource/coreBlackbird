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
 * Author: Sergen Eren
 */

#include "bvh/octree_build.h"

#include "render/mesh.h"
#include "render/object.h"

#include "util/util_algorithm.h"
#include "util/util_foreach.h"
#include "util/util_logging.h"
#include "util/util_progress.h"
#include "util/util_queue.h"
#include "util/util_simd.h"
#include "util/util_stack_allocator.h"
#include "util/util_time.h"

CCL_NAMESPACE_BEGIN

/* Constructor / Destructor */
OCTBuild::OCTBuild()
{
}
OCTBuild::~OCTBuild()
{
}

OCTNode *OCTBuild::build_root(vector<Object *> &objects)
{
    OCTNode * root = new OCTNode;
    
}

void OCTBuild::progress_update(){}

CCL_NAMESPACE_END
