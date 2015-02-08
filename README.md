# RayTracer #

[![Build Status](https://travis-ci.org/nikolaydio/raytracer.svg?branch=master)](https://travis-ci.org/nikolaydio/raytracer)

Screenshots available at https://drive.google.com/folderview?id=0B-aY1M-oaiRtaVE3TWJLM09wRFE
### Tasks ###
 * Texture support based on bilinear filtering(high priority).
 * Area light sampling(high priority).
 * Clean up the scene loading code. Make it print error message & line if broken scene desc.
 * Fresnel reflectance for scattering only(optional).

### Done Tasks ###
 * K-d tree accelerator (Naive implementation, good enough for now. Possible optimizations: Adaptive split and packing nodes in linear array)
 * Mesh loading using assimp
 * Scene loading from a basic text format(Draft code. Not final)
 * Multithreaded using OpenMP
 * Triangle Shape
 * Basic Image Sampling
 * Makefile for building under linux
 * BRDF Interface with Lambertian(diffuse) & specular implementations. (specular class done but require more testing)


### Building ###

#### Windows - Visual Studio ####
 * Download SDL2 into 3rdparty/
 * Download GLM into 3rdparty/
 * Download Google Test into 3rdparty/ (if you want to run the unit tests)
 * Download Open Asset Importer into 3rdparty/ (you need to build it)
 * Visual studio 2013 open .sln


#### Linux(Ubuntu) - Makefile ####
 * apt-get install libsdl2-dev
 * apt-get install libassimp-dev
 * Download GLM into 3rdparty/
 * make
