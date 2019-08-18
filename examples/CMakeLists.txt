add_executable(simple_example MACOSX_BUNDLE simple.c)
target_link_libraries(simple_example sokol)

sokol_app(cube cube.glsl cube.c)