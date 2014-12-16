# RayTracer #

### Tasks ###
 * BRDF Interface with Lambertian(diffuse), specular and compound implementations. (high priority)
 * Stratified Sampling support
 * Makefile for building under linux. (high priority)
 * Texture support. Linear filtering may be required if the result is looking too bad.
 * Fresnel reflectance and support for surface transmission(transparent objects) (optional)

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



### Compilling ###
 * Download SDL2 into 3rdparty/
 * Download GLM into 3rdparty/
 * Download Google Test into 3rdparty/ (if you want to run the unit tests)
 * Download Open Asset Importer into 3rdparty/
 * Visual studio 2013 open .sln