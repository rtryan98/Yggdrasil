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
