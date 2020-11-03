
# Rez deps paths

file(TO_CMAKE_PATH $ENV{BOOST_ROOT} BOOST_ROOT)
file(TO_CMAKE_PATH $ENV{BOOST_INCLUDE_DIR} BOOST_INCLUDEDIR)
file(TO_CMAKE_PATH $ENV{BOOST_INCLUDE_DIR} Boost_INCLUDE_DIR)


file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_LIBRARY_DIR} OPENJPEG_LIBRARY)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_INCLUDE_DIR} OPENJPEG_INCLUDE_DIR)
file(TO_CMAKE_PATH $ENV{LIBJPEG_TURBO_ROOT} JPEGTURBO_PATH)

# Cycles options

set(WITH_CPU_SSE ON)
set(WITH_CYCLES_STANDALONE_GUI OFF)
set(WITH_BLENDER OFF)

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

if(DEFINED ENV{REZ_EMBREE_BASE})
    message("WITH REZ EMBREE")
    set(WITH_CYCLES_EMBREE ON CACHE BOOL "Build Cycles with embree support" FORCE)
else()
    set(WITH_CYCLES_EMBREE OFF CACHE BOOL "Build Cycles with embree support" FORCE)
endif()

set(WITH_CYCLES_DEBUG OFF CACHE BOOL "Build Cycles with with extra debug capabilties" FORCE)

set(WITH_CYCLES_CUDA_BINARIES OFF CACHE BOOL "Build Cycles CUDA binaries" FORCE)

# TODO: These might not be set properly. Denoise still seems to print ON in console
set(WITH_CYCLES_OPENIMAGEDENOISE OFF CACHE BOOL "Build Cycles with OpenImageDenoise support" FORCE)

if(DEFINED ENV{REZ_GLOG_BASE} AND DEFINED ENV{REZ_GFLAGS_BASE})
    set(WITH_CYCLES_LOGGING ON CACHE BOOL "Build Cycles with logging support" FORCE)
    add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
endif()