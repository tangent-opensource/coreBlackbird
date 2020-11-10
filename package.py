# -*- coding: utf-8 -*-

name = 'cycles'

version = '1.13.0-ta.1.5.1'

authors = [
    'benjamin.skinner',
    'blender.foundation',
]

requires = [
    #'boost', # Technically houdini uses a different version which will clash with openvdb
    'glew',
    'oiio',
    'libjpeg_turbo-2.0.5',
    'opensubdiv',
    'ocio-1.1.1',
    'tbb-2019.U9',
    'openexr-2.4.0',
    'embree-3.8.0',
    #'openvdb-7.0.0', #'openvdb-6.2.1-houdini',
    
    # Only needed for logging
    'glog-0.4.0',
    'gflags-2.2.2',
]


@early()
def private_build_requires():
    
    # Tangent(bjs): Currently all variants are built with boost-1.69
    # However technically houdini should be built against boost-1.61.0
    # When it is resolved with HdCycles, it uses the correct boost
    common = [
        'boost-1.69.0',
    ]

    import sys
    if 'win' in str(sys.platform):
        return common + ['visual_studio']
    else:
        return common + ['gcc-7']

variants = [
    ['platform-windows', 'arch-x64', 'os-windows-10', 'oiio-1.8.9', 'opensubdiv-3.4.3'],
    ['platform-windows', 'arch-x64', 'os-windows-10', 'oiio-2.0.10-houdini', 'opensubdiv-3.3.3-houdini'],
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
