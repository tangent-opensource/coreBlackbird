# override options
set(WITH_CYCLES_OPENIMAGEDENOISE OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_OSL OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_STANDALONE_GUI OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_LOGGING OFF CACHE BOOL "" FORCE)

# compilation requirements
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)

# VFX platform requirements
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Houdini
message(STATUS "HOUDINI_ROOT: ${HOUDINI_ROOT}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(HOUDINI_DSOLIB "${HOUDINI_ROOT}/custom/houdini/dsolib/")
    set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
else()
    set(HOUDINI_DSOLIB ${HOUDINI_ROOT}/dsolib)
endif()
message(STATUS "HOUDINI_DSOLIB: ${HOUDINI_DSOLIB}")

set(HOUDINI_INCLUDE ${HOUDINI_ROOT}/toolkit/include)
message(STATUS "HOUDINI_INCLUDE: ${HOUDINI_INCLUDE}")

# not every library is being linked
set(CMAKE_BUILD_RPATH ${HOUDINI_DSOLIB})
set(CMAKE_INSTALL_RPATH ${HOUDINI_DSOLIB})

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
        NAMES
        hboost_filesystem
        hboost_filesystem-mt-x64
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(BOOST_REGEX_LIB
        NAMES
        hboost_regex
        hboost_regex-mt-x64
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(BOOST_SYSTEM_LIB
        NAMES
        hboost_system
        hboost_system-mt-x64
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(Boost_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(Boost_FILESYSTEM_LIBRARY_DEBUG ${BOOST_FILESYSTEM_LIB})
set(Boost_FILESYSTEM_LIBRARY_RELEASE ${BOOST_FILESYSTEM_LIB})
set(Boost_REGEX_LIBRARY_DEBUG ${BOOST_REGEX_LIB})
set(Boost_REGEX_LIBRARY_RELEASE ${BOOST_REGEX_LIB})
set(Boost_SYSTEM_LIBRARY_DEBUG ${BOOST_SYSTEM_LIB})
set(Boost_SYSTEM_LIBRARY_RELEASE ${BOOST_SYSTEM_LIB})

set(Boost_FOUND TRUE)

# OpenImageIO
find_library(OPENIMAGEIO_LIB
        NAMES OpenImageIO_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENIMAGEIO_LIBRARY ${OPENIMAGEIO_LIB})
set(OPENIMAGEIO_INCLUDE_DIR ${HOUDINI_INCLUDE})

# OpenEXR
find_library(OPENEXR_HALF_LIB
        NAMES Half_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_IEX_LIB
        NAMES Iex_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_ILMIMF_LIB
        NAMES IlmImf_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_ILMTHREAD_LIB
        NAMES IlmThread_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

find_library(OPENEXR_IMATH_LIB
        NAMES Imath_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENEXR_HALF_LIBRARY ${OPENEXR_HALF_LIB})
set(OPENEXR_IEX_LIBRARY ${OPENEXR_IEX_LIB})
set(OPENEXR_ILMIMF_LIBRARY ${OPENEXR_ILMIMF_LIB})
set(OPENEXR_ILMTHREAD_LIBRARY ${OPENEXR_ILMTHREAD_LIB})
set(OPENEXR_IMATH_LIBRARY ${OPENEXR_IMATH_LIB})
set(OPENEXR_ROOT_DIR ${HOUDINI_DSOLIB})
set(OPENEXR_INCLUDE_DIR ${HOUDINI_INCLUDE})

# OpenShadingLanguage

# OpenColorIO
find_path(OPENCOLORIO_HEADER
        NAMES OpenColorIO.h
        PATHS ${HOUDINI_INCLUDE}/OpenColorIO/
        NO_DEFAULT_PATH
        )

find_library(OPENCOLORIO_LIB
        NAMES OpenColorIO_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENCOLORIO_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(OPENCOLORIO_OPENCOLORIO_LIBRARY ${OPENCOLORIO_LIB})

add_definitions(-DOCIO_NAMESPACE=v1_sidefx)

# tiff
find_library(TIFF_LIB
        NAMES tiff
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(TIFF_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(TIFF_LIBRARY ${TIFF_LIB})

# png
find_library(PNG_LIB
        NAMES
        png
        png16
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        )

set(PNG_INCLUDE_DIRS ${HOUDINI_INCLUDE})
set(PNG_PNG_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(PNG_LIBRARY ${PNG_LIB})

# OpenSubdiv
find_library(OSD_OSDGPU_LIB
        NAMES
        osdGPU
        osdGPU_md
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )


find_library(OSD_OSDCPU_LIB
        NAMES
        osdCPU
        osdCPU_md
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENSUBDIV_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(OPENSUBDIV_OSDGPU_LIBRARY ${OSD_OSDGPU_LIB})
set(OPENSUBDIV_OSDCPU_LIBRARY ${OSD_OSDCPU_LIB})

# OpenVDB
find_library(OPENVDB_LIB
        NAMES openvdb_sesi
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(OPENVDB_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(OPENVDB_LIBRARY ${OPENVDB_LIB})

# blosc
find_library(BLOSC_LIB
        NAMES blosc
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(BLOSC_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(BLOSC_LIBRARY ${BLOSC_LIB})

# tbb
find_library(TBB_LIB
        NAMES tbb
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(TBB_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(TBB_LIBRARY ${TBB_LIB})

# glew
find_library(GLEW_LIB
        NAMES GLEW
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(GLEW_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(GLEW_LIBRARY ${GLEW_LIB})

# jpeg
find_library(JPEG_LIB
        NAMES jpeg
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(JPEG_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(JPEG_LIBRARY ${JPEG_LIB})

# libz
find_library(Z_LIB
        NAMES
        z
        zdll
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(ZLIB_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(ZLIB_LIBRARY ${Z_LIB})

# embree
find_library(EMBREE_LIB
        NAMES embree_sidefx
        PATHS ${HOUDINI_DSOLIB}
        NO_DEFAULT_PATH
        REQUIRED
        )

set(EMBREE_INCLUDE_DIR ${HOUDINI_INCLUDE})
set(EMBREE_EMBREE3_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_AVX_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_AVX2_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_LEXERS_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_MATH_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_SIMD_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_SYS_LIBRARY ${EMBREE_LIB})
set(EMBREE_EMBREE_TASKING_LIBRARY ${EMBREE_LIB})
