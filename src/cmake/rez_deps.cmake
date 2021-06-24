
# Rez deps paths

file(TO_CMAKE_PATH $ENV{BOOST_ROOT} BOOST_ROOT)
file(TO_CMAKE_PATH $ENV{BOOST_INCLUDE_DIR} BOOST_INCLUDEDIR)
file(TO_CMAKE_PATH $ENV{BOOST_INCLUDE_DIR} Boost_INCLUDE_DIR)

if(DEFINED ENV{BLOSC_INCLUDE_DIR})
    file(TO_CMAKE_PATH $ENV{BLOSC_INCLUDE_DIR} BLOSC_INCLUDE_DIR)
endif()

file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_LIBRARY_DIR} OPENJPEG_LIBRARY)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_INCLUDE_DIR} OPENJPEG_INCLUDE_DIR)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_ROOT} JPEGTURBO_PATH)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_LIBRARY_DIR} JPEG_LIBRARY)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_INCLUDE_DIR} JPEG_INCLUDE_DIR)

find_package(HBoost)

# Cycles options

set(WITH_CPU_SSE ON)
set(WITH_CYCLES_STANDALONE_GUI OFF)
set(WITH_BLENDER OFF)

option(WITH_CYCLES_STANDALONE_GUI   "Build Cycles standalone with GUI" OFF)

# Linux doies not need OpenGL Support (Windows likely doesn't either)
# so we added ifdef's throughout to remove GL deps
if (MSVC)
  set(USE_OPENGL TRUE)
  add_definitions(-DUSE_OPENGL)
endif()

if (UNIX)
  add_definitions(-DCMAKE_POSITION_INDEPENDENT_CODE=ON)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(DEFINED ENV{REZ_OSL_BASE})
    message("WITH REZ OSL")
    set(WITH_CYCLES_OSL ON CACHE BOOL "Build Cycles with OSL support" FORCE)
else()
    set(WITH_CYCLES_OSL OFF CACHE BOOL "Build Cycles with OSL support" FORCE)
endif()

if(DEFINED ENV{REZ_OPENCOLORIO_BASE})
    message("WITH REZ OCIO")
    set(WITH_CYCLES_OPENCOLORIO ON CACHE BOOL "Build Cycles with OpenColorIO support" FORCE)
else()
    set(WITH_CYCLES_OPENCOLORIO OFF CACHE BOOL "Build Cycles with OpenColorIO support" FORCE)
endif()

if(DEFINED ENV{REZ_OPENSUBDIV_BASE})
    message("WITH REZ OPENSUBDIV")
    set(WITH_CYCLES_OPENSUBDIV ON CACHE BOOL "Build Cycles with OpenSubdiv support" FORCE)
else()
    set(WITH_CYCLES_OPENSUBDIV OFF CACHE BOOL "Build Cycles with OpenSubdiv support" FORCE)
endif()

if(DEFINED ENV{REZ_OPENVDB_BASE})
    message("WITH REZ OPENVDB")
    set(WITH_CYCLES_OPENVDB ON CACHE BOOL "Build Cycles with OpenVDB support" FORCE)
else()
    set(WITH_CYCLES_OPENVDB OFF CACHE BOOL "Build Cycles with OpenVDB support" FORCE)
endif()

if(DEFINED ENV{REZ_NANOVDB_BASE})
    message("WITH REZ NANOVDB")
    set(WITH_NANOVDB ON CACHE BOOL "Build Cycles with NanoVDB support" FORCE)
	file(TO_CMAKE_PATH $ENV{NANOVDB_INCLUDE_DIR} NANOVDB_INCLUDE_DIR)
else()
    set(WITH_NANOVDB OFF CACHE BOOL "Build Cycles with NanoVDB support" FORCE)
endif()

if(DEFINED ENV{REZ_EMBREE_BASE})
    message("WITH REZ EMBREE")
    set(WITH_CYCLES_EMBREE ON CACHE BOOL "Build Cycles with embree support" FORCE)
else()
    set(WITH_CYCLES_EMBREE OFF CACHE BOOL "Build Cycles with embree support" FORCE)
endif()

if(DEFINED ENV{REZ_OIDN_BASE})
    message("WITH REZ OIDN")
    set(WITH_CYCLES_OPENIMAGEDENOISE ON CACHE BOOL "Build Cycles with OpenImageDenoise support" FORCE)
else()
    set(WITH_CYCLES_OPENIMAGEDENOISE OFF CACHE BOOL "Build Cycles with OpenImageDenoise support" FORCE)
endif()

set(WITH_CYCLES_DEBUG OFF CACHE BOOL "Build Cycles with with extra debug capabilties" FORCE)

set(WITH_CYCLES_CUDA_BINARIES OFF CACHE BOOL "Build Cycles CUDA binaries" FORCE)

# TODO: These might not be set properly. Denoise still seems to print ON in console
set(WITH_CYCLES_OPENIMAGEDENOISE OFF CACHE BOOL "Build Cycles with OpenImageDenoise support" FORCE)

if(DEFINED ENV{REZ_GLOG_BASE} AND DEFINED ENV{REZ_GFLAGS_BASE})
    set(WITH_CYCLES_LOGGING ON CACHE BOOL "Build Cycles with logging support" FORCE)
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
endif()
