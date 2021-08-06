# -*- coding: utf-8 -*-

name = 'cycles'


version = '1.13.0-ta.1.14.7'

authors = [
    'benjamin.skinner',
    'blender.foundation',
]

requires = [
    'glew', # Specified in opensubdiv... (Revist this later)
    'oiio',
    'libjpeg_turbo-2.0.5',
    'opensubdiv',
    'ocio-1.1.1',
    'tbb-2019.U9',
    'openexr-2.4.0',
    'embree-3.8.0',
    'nanovdb-29.3.0',

    # Only needed for logging
    'glog-0.4.0',
    'gflags-2.2.2',
]

@early()
def private_build_requires():
    import sys
    if 'win' in str(sys.platform):
        return ['cmake-3.18<3.20', 'visual_studio',]
    else:
        return ['cmake-3.18<3.20', 'gcc-6',]

variants = [
    ['platform-windows', 'arch-x64', 'os-windows-10', 'oiio-1.8.9', 'opensubdiv-3.4.3', 'boost-1.69.0', 'openvdb-7.0.0'],
    ['platform-windows', 'arch-x64', 'os-windows-10', 'oiio-2.0.10-houdini', 'opensubdiv-3.4.3-houdini', 'boost-1.65.1', 'openvdb-7.2.2-houdini'],
    ['platform-linux', 'arch-x86_64', 'os-centos-7', 'oiio-2.0.10-houdini', 'opensubdiv-3.4.3-houdini', 'boost-1.65.1', 'openvdb-7.2.2-houdini'],
]

# Using an external openvdb build caused instant crashes when hdcycles ran inside of houdini.
# Ben might have missed something, however tried and failed.
# Because of this, we need to use Houdini's openvdb version. And only 18.5 used a compatible version of openvdb
# as such, only houdini-18.5 is compatible with hdcycles volumes
# Also, because openvdb-7.1.0-houdini needs hboost, we have to modify some cmake lists to use hboost includes....

hashed_variants = True

build_system = "cmake"

def commands():

    if building:
        env.CMAKE_PREFIX_PATH.prepend('{}/lib/cmake'.format(env.REZ_GLOG_ROOT))
        env.CMAKE_PREFIX_PATH.prepend('{}/lib/cmake'.format(env.REZ_GFLAGS_ROOT))
        env.CMAKE_PREFIX_PATH.prepend('{root}/lib/cmake')

     # Split and store version and package version
    split_versions = str(version).split('-')
    env.CYCLES_VERSION.set(split_versions[0])
    env.CYCLES_PACKAGE_VERSION.set(split_versions[1])

    env.CYCLES_ROOT.set( "{root}" )
    env.CYCLES_LIB_DIR.set( "{root}/lib" )
    env.CYCLES_INCLUDE_DIR.set( "{root}/include" )

    env.PATH.append( "{root}/bin" )
