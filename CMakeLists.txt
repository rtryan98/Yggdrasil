# Copyright 2022 Robert Ryan. See Licence.md.

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(yggdrasil.cmake)

set(YGG_PROJECT "Yggdrasil")
set(YGG_LIBRARY "Yggdrasil-Engine")
set(YGG_APPLICATION "Yggdrasil-Application")
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

project(${YGG_PROJECT})
find_package(Vulkan REQUIRED)
add_subdirectory("vendor/volk")

file(GLOB_RECURSE "YGG_LIBRARY_SRC" "src/ygg/*.h" "src/ygg/*.cpp")
ygg_group_files("${YGG_LIBRARY_SRC}")
add_library(${YGG_LIBRARY} ${YGG_LIBRARY_SRC})
ygg_set_output_dirs(${YGG_LIBRARY})

target_include_directories(${YGG_LIBRARY} PUBLIC
    "src"
    "vendor/glm"
    "vendor/concurrentqueue")
target_link_libraries(${YGG_LIBRARY} PUBLIC
    volk_headers)

target_compile_definitions(${YGG_LIBRARY} PRIVATE VK_USE_PLATFORM_WIN32_KHR=1)
target_compile_definitions(${YGG_LIBRARY} PUBLIC
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS)

file(GLOB_RECURSE "YGG_APPLICATION_SRC" "src/ygg_application/*.h" "src/ygg_application/*.cpp")
ygg_group_files("${YGG_APPLICATION_SRC}")
add_executable(${YGG_APPLICATION} ${YGG_APPLICATION_SRC})
ygg_set_output_dirs(${YGG_APPLICATION})
target_link_libraries(${YGG_APPLICATION} PRIVATE ${YGG_LIBRARY})

if(MSVC)
    target_compile_options(${YGG_LIBRARY} PRIVATE
        "/W4" "/MP" "/std:c++latest" "/WX" "/wd26812")
    target_compile_options(${YGG_APPLICATION} PRIVATE
        "/W4" "/MP" "/std:c++latest" "/WX" "/wd26812")
    target_compile_definitions(${YGG_LIBRARY} PRIVATE
        "$<$<CONFIG:Debug>:YGG_VULKAN_VALIDATION=1; YGG_DEBUG=1>")
    target_compile_definitions(${YGG_LIBRARY} PRIVATE
        "$<$<CONFIG:RelWithDebInfo>:YGG_VULKAN_VALIDATION=1; YGG_DEBUG=0>")
    target_compile_definitions(${YGG_LIBRARY} PRIVATE
        "$<$<CONFIG:Release>:YGG_VULKAN_VALIDATION=0; YGG_DEBUG=0>")
    set_target_properties(${YGG_LIBRARY} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${YGG_APPLICATION})
endif()
