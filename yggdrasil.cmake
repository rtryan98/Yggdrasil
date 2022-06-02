# Copyright 2021 Robert Ryan. See LICENCE.md.

macro(ygg_set_output_dirs TARGET)
    # static libraries
    set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug/")
    set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo/")
    set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release/")
    # executables
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug/")
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo/")
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release/")
    # dynamic libraries
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/debug/")
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/relwithdebinfo/")
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/release/")
endmacro()

macro(ygg_group_files FILES)
    foreach(item IN ITEMS ${FILES})
        get_filename_component(src_path "${item}" PATH)
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" group_path "${src_path}")
        string(REPLACE "/" "\\" group_path "${group_path}")
        source_group("${group_path}" FILES "${item}")
    endforeach()
endmacro()

macro(ygg_configure_project_msvc TARGET)
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE
        "/W4" "/MP" "/std:c++20" "/WX" "/wd26812")
        target_compile_definitions(${TARGET} PRIVATE
        "$<$<CONFIG:Debug>:YGG_VULKAN_VALIDATION=1; YGG_DEBUG=1; YGG_VULKAN_NAMES=1>")
        target_compile_definitions(${TARGET} PRIVATE
        "$<$<CONFIG:RelWithDebInfo>:YGG_VULKAN_VALIDATION=1; YGG_DEBUG=0; YGG_VULKAN_NAMES=1>")
        target_compile_definitions(${TARGET} PRIVATE
        "$<$<CONFIG:Release>:YGG_VULKAN_VALIDATION=0; YGG_DEBUG=0; YGG_VULKAN_NAMES=0>")
        set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    endif()
endmacro()
