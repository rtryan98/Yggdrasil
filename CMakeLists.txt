# Copyright 2022 Robert Ryan. See Licence.md.

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
include(yggdrasil.cmake)

set(YGG_PROJECT "yggdrasil")
set(YGG_LIBRARY "yggdrasil_engine")
set(YGG_MINI_SAMPLE_BASE "yggdrasil_mini_sample_base")
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

project(${YGG_PROJECT})
find_package(Vulkan REQUIRED)

add_subdirectory("vendor/volk")

add_subdirectory("vendor/vma")

set(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_GLSLANG_JS OFF)
set(ENABLE_HLSL OFF)
set(ENABLE_GLSLANG_WEBMIN OFF)
set(ENABLE_SPVREMAPPER OFF)

add_subdirectory("vendor/glslang")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/vulkan_profiles.hpp)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated)
    execute_process(COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vulkan-profiles/scripts/genvp.py
        -registry ${CMAKE_CURRENT_SOURCE_DIR}/vendor/vulkan-docs/xml/vk.xml
        -profiles ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan
        -outIncDir ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated
        -outSrcDir ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated
        -outSchema ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/schema.json)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/schema.json)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/schema.json)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/vulkan_profiles.h)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/vulkan_profiles.h)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/vulkan_profiles.cpp)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg/vulkan/generated/vulkan_profiles.cpp)
endif()

file(GLOB_RECURSE "YGG_LIBRARY_SRC" "src/ygg/*.h" "src/ygg/*.cpp" "src/ygg/*.json" "src/ygg/*.hpp")
ygg_group_files("${YGG_LIBRARY_SRC}")
add_library(${YGG_LIBRARY} ${YGG_LIBRARY_SRC})
ygg_set_output_dirs(${YGG_LIBRARY})
target_include_directories(${YGG_LIBRARY} PUBLIC
    "src"
    "vendor/glm"
    "vendor/concurrentqueue")
target_link_libraries(${YGG_LIBRARY} PUBLIC
    volk_headers
    VulkanMemoryAllocator
    glslang
    SPIRV
    rpcrt4.lib
    )
ygg_configure_project_msvc(${YGG_LIBRARY})

target_compile_definitions(${YGG_LIBRARY} PRIVATE
    VK_USE_PLATFORM_WIN32_KHR=1)
target_compile_definitions(${YGG_LIBRARY} PUBLIC
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    YGG_MAX_FRAMES_IN_FLIGHT=3
    VK_NO_PROTOTYPES)

file(GLOB_RECURSE "YGG_MINI_SAMPLE_SRC" "src/ygg_mini_sample_base/*.h" "src/ygg_mini_sample_base/*.cpp")
ygg_group_files("${YGG_MINI_SAMPLE_SRC}")
add_library(${YGG_MINI_SAMPLE_BASE} ${YGG_MINI_SAMPLE_SRC})
ygg_set_output_dirs(${YGG_MINI_SAMPLE_BASE})
target_link_libraries(${YGG_MINI_SAMPLE_BASE} PUBLIC ${YGG_LIBRARY})
ygg_configure_project_msvc(${YGG_MINI_SAMPLE_BASE})

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${YGG_APPLICATION})
endif()

option(YGG_BUILD_SAMPLES ON)
function(ygg_create_sample EXAMPLE_NAME)
    set(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src/ygg_mini_samples/${EXAMPLE_NAME})
    file(GLOB_RECURSE EXAMPLE_SOURCE ${EXAMPLE_FOLDER}/*.cpp ${EXAMPLE_FOLDER}/*.h)
    set(EXAMPLE_PROJECT_NAME yggdrasil_${EXAMPLE_NAME})
    add_executable(${EXAMPLE_PROJECT_NAME} ${EXAMPLE_SOURCE})
    ygg_group_files("${EXAMPLE_SOURCE}")
    ygg_set_output_dirs(${EXAMPLE_PROJECT_NAME})
    ygg_configure_project_msvc(${EXAMPLE_PROJECT_NAME})
    target_link_libraries(${EXAMPLE_PROJECT_NAME} PUBLIC ${YGG_MINI_SAMPLE_BASE})
endfunction()

set(YGG_EXAMPLES
    vk_hello_cube)

foreach(EXAMPLE ${YGG_EXAMPLES})
    ygg_create_sample(${EXAMPLE})
endforeach()
