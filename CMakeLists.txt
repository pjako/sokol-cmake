cmake_minimum_required(VERSION 3.1)
project(sokol-cmake)
# add external dependencies
add_subdirectory(deps)

# include function which calls the shader generator sokol-shd
include(cmake/shd.cmake)
# include function for creating a windowed sokol app, depends on sokol headers
include(cmake/sokol.cmake)


if (APPLE)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES")
    set(CMAKE_EXE_LINKER_FLAGS "-ObjC -dead_strip -lpthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
endif()

include_directories(src)

option(BUILD_EXAMPLES "Build examples" TRUE)
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


