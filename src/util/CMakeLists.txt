# Copyright 2011-2020 Blender Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(INC
  ..
  ../../glew-mx
)

set(INC_SYS
  ${GLEW_INCLUDE_DIR}
)

set(SRC
  util_aligned_malloc.cpp
  util_debug.cpp
  util_ies.cpp
  util_logging.cpp
  util_math_cdf.cpp
  util_md5.cpp
  util_murmurhash.cpp
  util_path.cpp
  util_profiling.cpp
  util_string.cpp
  util_simd.cpp
  util_system.cpp
  util_task.cpp
  util_thread.cpp
  util_time.cpp
  util_transform.cpp
  util_windows.cpp
)

if(CYCLES_STANDALONE_REPOSITORY)
  message(STATUS "utils linking with numaapi")
  set(LIB ${TBB_LIBRARIES} extern_numaapi)
else()
  set(LIB ${TBB_LIBRARIES})
endif()

if(WITH_CYCLES_STANDALONE)
  if(WITH_CYCLES_STANDALONE_GUI)
    list(APPEND SRC
      util_view.cpp
    )
  endif()
endif()

if(CYCLES_STANDALONE_REPOSITORY)
  list(APPEND INC_SYS
    ../../third_party/numaapi/include
  )
else()
  list(APPEND INC_SYS
    ../../numaapi/include
  )
endif()

set(SRC_HEADERS
  util_algorithm.h
  util_aligned_malloc.h
  util_args.h
  util_array.h
  util_atomic.h
  util_boundbox.h
  util_debug.h
  util_defines.h
  util_deque.h
  util_disjoint_set.h
  util_guarded_allocator.cpp
  util_foreach.h
  util_function.h
  util_guarded_allocator.h
  util_half.h
  util_hash.h
  util_ies.h
  util_image.h
  util_image_impl.h
  util_list.h
  util_logging.h
  util_map.h
  util_math.h
  util_math_cdf.h
  util_math_fast.h
  util_math_intersect.h
  util_math_float2.h
  util_math_float3.h
  util_math_float4.h
  util_math_int2.h
  util_math_int3.h
  util_math_int4.h
  util_math_matrix.h
  util_md5.h
  util_murmurhash.h
  util_openimagedenoise.h
  util_opengl.h
  util_openvdb.h
  util_optimization.h
  util_param.h
  util_path.h
  util_profiling.h
  util_progress.h
  util_projection.h
  util_queue.h
  util_rect.h
  util_set.h
  util_simd.h
  util_avxf.h
  util_avxb.h
  util_avxi.h
  util_semaphore.h
  util_sseb.h
  util_ssef.h
  util_ssei.h
  util_stack_allocator.h
  util_static_assert.h
  util_stats.h
  util_string.h
  util_system.h
  util_task.h
  util_tbb.h
  util_texture.h
  util_thread.h
  util_time.h
  util_transform.h
  util_types.h
  util_types_float2.h
  util_types_float2_impl.h
  util_types_float3.h
  util_types_float3_impl.h
  util_types_float4.h
  util_types_float4_impl.h
  util_types_float8.h
  util_types_float8_impl.h
  util_types_int2.h
  util_types_int2_impl.h
  util_types_int3.h
  util_types_int3_impl.h
  util_types_int4.h
  util_types_int4_impl.h
  util_types_uchar2.h
  util_types_uchar2_impl.h
  util_types_uchar3.h
  util_types_uchar3_impl.h
  util_types_uchar4.h
  util_types_uchar4_impl.h
  util_types_uint2.h
  util_types_uint2_impl.h
  util_types_uint3.h
  util_types_uint3_impl.h
  util_types_uint4.h
  util_types_uint4_impl.h
  util_types_ushort4.h
  util_types_vector3.h
  util_types_vector3_impl.h
  util_unique_ptr.h
  util_vector.h
  util_version.h
  util_view.h
  util_windows.h
  util_xml.h
)

include_directories(${INC})
include_directories(SYSTEM ${INC_SYS})

add_definitions(${GL_DEFINITIONS})

cycles_add_library(cycles_util "${LIB}" ${SRC} ${SRC_HEADERS})

if(${WITH_CYCLES_STANDALONE_GUI})
  target_link_libraries(cycles_util GL GLU)
endif()