cmake_minimum_required(VERSION 3.1)
project(sokol-cmake)
add_definitions(-DRX_DX12_ENABLED)
if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
elseif(ANDROID)
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX AND NOT APPLE) # i.e.: Linux
    if(BUILD_WSI_XCB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    endif()
    if(BUILD_WSI_XLIB_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR -DVK_USE_PLATFORM_XLIB_XRANDR_EXT)
    endif()
    if(BUILD_WSI_WAYLAND_SUPPORT)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    endif()
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

include_directories(deps/vulkan/include)

if (APPLE)
    add_definitions(-DSOKOL_METAL)
    if (IOS)
        set(slang "metal_ios:metal_sim")
    else()
        set(slang "metal_macos")
    endif()
elseif (ANDROID)
    add_definitions(-DSOKOL_GLES3)
    set(slang "glsl300es")
elseif (WIN32)
    add_definitions(-DSOKOL_D3D11)
    set(slang "hlsl5")
    message(STATUS "using D3D11")
endif()

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


set(HEADER_FILES
    src/rx_dx12.h
    src/rx_renderer.h
    src/rx_vkrenderer.h
    src/rx_internal.h
    src/rx_dx12_helper.h
)
set(SOURCE_FILES
    src/rx_renderer.c
    src/rx_dx12.c
    src/vk_enum_string_helper.h
    src/vk_mem_alloc.cc
    src/vk_mem_alloc.h
    src/rx_dx12_helper.cc
)

add_library(RXRenderer STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)
target_include_directories(RXRenderer PUBLIC src/)

add_executable(App WIN32
	app.c
)

# target_include_directories(App
#     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
# )

target_link_libraries(App
    RXRenderer
    d3d12.lib
    dxgi.lib
    dxguid.lib
)


option(BUILD_EXAMPLES "Build examples" TRUE)
if (BUILD_EXAMPLES)
#    add_subdirectory(examples)
endif()


#vulkan_app(my_app src/rx_renderer.c src/vk_mem_alloc.cc)

