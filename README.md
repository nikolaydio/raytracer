# RayTracer #

[![Build Status](https://travis-ci.org/nikolaydio/raytracer.svg?branch=master)](https://travis-ci.org/nikolaydio/raytracer)
[![Build Status](https://travis-ci.org/nikolaydio/raytracer.svg?branch=develop)](https://travis-ci.org/nikolaydio/raytracer)

### Intro ###
This is a student project that aims to prove that physically based rendering is not some "black magic". It intends to create a minimal&simplified rendering engine usable by students and enthusiasts. 


Screenshots(early ones) available at https://drive.google.com/folderview?id=0B-aY1M-oaiRtaVE3TWJLM09wRFE

If you got this document from other source than github - you can view the most up to date version at https://github.com/nikolaydio/raytracer

### Features ###
 * Material system with emit, diffuse, glossy and specular options
 * Support for most common image formats by stb_image
 * Path tracing(single light sampling + cosine weighted distribution)
 * Perspective camera
 * 3D models by Open Asset Importer
 * Kd-tree acceleration for both the scene and the 3d models
 * Multithreaded by OpenMP
 * Interactive rendering process by SDL2
 * JSON-like configuration files for scene and camera descriptions
 * Multiplatform (tested on linux and windows)

### Bugs ###
Report any bugs you find in the issue tracking system or send a pull request with fix. I doubt anyone cares... but anyways...

### Building ###

The notes below assume you don't have "3rdparty" folder. If your distribution contains such folder then it contains 32bit windows prebuilt libraries with the following differences:
 * The assimp directory that is provided contains precompiled binaries by me. The original ones are broken(see additional tips).
 * Removed 64bit SDL2.

#### Windows - Visual Studio ####
 * Download SDL 2.0.3 into 3rdparty/
 * Download GLM 0.9.4 into 3rdparty/
 * Download Google Test into 3rdparty/ (if you want to run the unit tests)
 * Download Open Asset Importer 3.1 into 3rdparty/ (you need to build it)
 * Visual studio 2013 open .sln

Additional tips:
 * OpenAssetImporter can easily be built by CMake. You should certainly build it yourself - https://github.com/assimp/assimp/issues/302
 * Newer GLM version dropped backward compatibilty with 0.9.4.
 * If you don't have visual studio 2013, you may be able to build the project using mingw or cygwin.(Not tested yet)

#### Linux(Ubuntu 14.04+) - Makefile ####
 * apt-get install libsdl2-dev
 * apt-get install libassimp-dev
 * Download GLM 0.9.4 into 3rdparty/
 * make

Additional tips:
 * Older ubuntu versions ship with old incompatible versions of the dependencies.
 * A C++ 11 compiler is required. GCC 4.8 gets the job done. Earlier releases may not be able to build the project.
