# RayTracer #

### Tasks ###
 * Stratified Sampling support(+integrate it in the codebase)
 * Texture support. Linear filtering may be required if the result is looking too bad.
 * Fresnel reflectance and support for surface transmission(transparent objects) (optional)
 * Area light sampling(high priority)

### Not exactly defined tasks ###
 * Research how to build a proper path integrator(ongoing)
 * Choose a good resource management model (Asset storage model).

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
 * if you still get compile time errors, try to
  * apt-get install linux-libc-dev
  * apt-get install g++-4.8-multilib (your gcc version here)
