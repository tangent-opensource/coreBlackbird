# -*- coding: utf-8 -*-

name = 'cycles'

version = '1.13.0-ta.1.0.0'

authors = [
    'benjamin.skinner',
    'blender.foundation',
]

requires = [
    'python-3',
    'glew-2.0.0',
    'boost-1.69.0',
    'oiio-1.8.9',
    'libjpeg_turbo-2.0.5',
    'embree-3.8.0',
    'opensubdiv-3.4.3',
    'openvdb-7.0.0',
    'tbb-2019',
    #'osl-1.10.9',
    #'nvidia',
]


@early()
def private_build_requires():
    import sys
    if 'win' in str(sys.platform):
        return ['visual_studio']
    else:
        return ['gcc-7']

variants = [
    ['platform-windows', 'arch-x64', 'os-windows-10'],
    #['platform-linux', 'arch-x64'],
]

build_system = "cmake"

def commands():

     # Split and store version and package version
    split_versions = str(version).split('-')
    env.CYCLES_VERSION.set(split_versions[0])
    env.CYCLES_PACKAGE_VERSION.set(split_versions[1])

    env.CYCLES_ROOT.set( "{root}" )
    env.CYCLES_LIB_DIR.set( "{root}/lib" )
    env.CYCLES_INCLUDE_DIR.set( "{root}/include" )

    env.PATH.append( "{root}/bin" )
