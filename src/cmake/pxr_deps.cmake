# override options
set(WITH_CYCLES_OPENIMAGEDENOISE OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_OSL OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_STANDALONE_GUI OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_LOGGING OFF CACHE BOOL "" FORCE)

# compilation requirements
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)

# VFX platform requirements

# Pixar USD
message(STATUS "USD_ROOT: ${USD_ROOT}")
set(USD_LIB ${USD_ROOT}/lib)
set(USD_INCLUDE ${USD_ROOT}/include)

# not every library is being linked
set(CMAKE_BUILD_RPATH ${USD_ROOT}/dsolib)
set(CMAKE_INSTALL_RPATH ${USD_ROOT}/dsolib)


# hboost
# override boost searching and inject hboost
macro(find_package)
    if(${ARGV0} STREQUAL "OpenJPEG")
        message(STATUS "Skipping OpenJPEG")
    elseif(${ARGV0} STREQUAL "Boost")
        message(STATUS "Skipping Boost")
    else()
        message(STATUS "Searching package: ${ARGV}")
        _find_package(${ARGV})
    endif()
endmacro()

find_library(BOOST_FILESYSTEM_LIB
        NAMES boost_filesystem
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )
message(STATUS "boost-filesystem: ${BOOST_FILESYSTEM_LIB}")

find_library(BOOST_REGEX_LIB
        NAMES boost_regex
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )
message(STATUS "boost-regex: ${BOOST_REGEX_LIB}")

find_library(BOOST_SYSTEM_LIB
        NAMES boost_system
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )
message(STATUS "boost-system: ${BOOST_REGEX_LIB}")

set(Boost_INCLUDE_DIR ${USD_INCLUDE})
set(Boost_FILESYSTEM_LIBRARY_DEBUG ${BOOST_FILESYSTEM_LIB})
set(Boost_FILESYSTEM_LIBRARY_RELEASE ${BOOST_FILESYSTEM_LIB})
set(Boost_REGEX_LIBRARY_DEBUG ${BOOST_REGEX_LIB})
set(Boost_REGEX_LIBRARY_RELEASE ${BOOST_REGEX_LIB})
set(Boost_SYSTEM_LIBRARY_DEBUG ${BOOST_SYSTEM_LIB})
set(Boost_SYSTEM_LIBRARY_RELEASE ${BOOST_SYSTEM_LIB})

set(Boost_FOUND TRUE)

# OpenImageIO
find_library(OPENIMAGEIO_LIB
        NAMES OpenImageIO
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENIMAGEIO_LIBRARY ${OPENIMAGEIO_LIB})
set(OPENIMAGEIO_INCLUDE_DIR ${USD_INCLUDE})

# OpenEXR
find_library(OPENEXR_HALF_LIB
        NAMES Half
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_IEX_LIB
        NAMES Iex Iex-2_2
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_ILMIMF_LIB
        NAMES IlmImf IlmImf-2_2
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_ILMTHREAD_LIB
        NAMES IlmThread IlmThread-2_2
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_IMATH_LIB
        NAMES Imath Imath-2_2
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENEXR_HALF_LIBRARY ${OPENEXR_HALF_LIB})
set(OPENEXR_IEX_LIBRARY ${OPENEXR_IEX_LIB})
set(OPENEXR_ILMIMF_LIBRARY ${OPENEXR_ILMIMF_LIB})
set(OPENEXR_ILMTHREAD_LIBRARY ${OPENEXR_ILMTHREAD_LIB})
set(OPENEXR_IMATH_LIBRARY ${OPENEXR_IMATH_LIB})
set(OPENEXR_ROOT_DIR ${USD_LIB})
set(OPENEXR_INCLUDE_DIR ${USD_INCLUDE})

# OpenShadingLanguage

# OpenColorIO
find_path(OPENCOLORIO_HEADER
        NAMES OpenColorIO.h
        PATHS ${USD_INCLUDE}/OpenColorIO/
        NO_DEFAULT_PATH
        )

find_library(OPENCOLORIO_LIB
        NAMES OpenColorIO
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENCOLORIO_INCLUDE_DIR ${USD_INCLUDE})
set(OPENCOLORIO_OPENCOLORIO_LIBRARY ${OPENCOLORIO_LIB})

# tiff
find_library(TIFF_LIB
        NAMES tiff
        PATHS ${USD_LIB}
        REQUIRED
        )

set(TIFF_INCLUDE_DIR ${USD_INCLUDE})
set(TIFF_LIBRARY ${TIFF_LIB})

# png
find_library(PNG_LIB
        NAMES png
        PATHS ${USD_LIB}
        REQUIRED
        )

set(PNG_INCLUDE_DIRS ${USD_INCLUDE})
set(PNG_PNG_INCLUDE_DIR ${USD_INCLUDE})
set(PNG_LIBRARY ${PNG_LIB})

# OpenSubdiv
find_library(OSD_OSDGPU_LIB
        NAMES osdGPU
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )


find_library(OSD_OSDCPU_LIB
        NAMES osdCPU
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENSUBDIV_INCLUDE_DIR ${USD_INCLUDE})
set(OPENSUBDIV_OSDGPU_LIBRARY ${OSD_OSDGPU_LIB})
set(OPENSUBDIV_OSDCPU_LIBRARY ${OSD_OSDCPU_LIB})

# OpenVDB
find_library(OPENVDB_LIB
        NAMES openvdb
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENVDB_INCLUDE_DIR ${USD_INCLUDE})
set(OPENVDB_LIBRARY ${OPENVDB_LIB})

# blosc
find_library(BLOSC_LIB
        NAMES blosc
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(BLOSC_INCLUDE_DIR ${USD_INCLUDE})
set(BLOSC_LIBRARY ${BLOSC_LIB})

# tbb
find_library(TBB_LIB
        NAMES tbb
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(TBB_INCLUDE_DIR ${USD_INCLUDE})
set(TBB_LIBRARY ${TBB_LIB})
add_definitions(-DTBB_INTERFACE_VERSION_MAJOR)

# jpeg
find_library(JPEG_LIB
        NAMES jpeg
        PATHS ${USD_LIB}
        REQUIRED
        )

set(JPEG_INCLUDE_DIR ${USD_INCLUDE})
set(JPEG_LIBRARY ${JPEG_LIB})

# embree
find_library(EMBREE_LIB
        NAMES embree embree3
        PATHS ${USD_LIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(EMBREE_INCLUDE_DIR ${USD_INCLUDE})
set(EMBREE_EMBREE3_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_AVX_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_AVX2_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_LEXERS_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_MATH_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_SIMD_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_SYS_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_TASKING_LIBRARY ${EMBREE_LIB})