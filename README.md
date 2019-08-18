# Pure cmake sokol project template
Sokol with cmake build system
+ Requires cmake 3.1
+ Includes a generator to compile sokol glsl shaders

Build on OSX with:
````
mkdir build && cd build && cmake -G Xcode .. 
````

Cmake example to build a sokol program
````
sokol_app(cube cube.glsl cube.c)
````

#Updates

18/08/2020 - Initial version, only supports OSX currently
