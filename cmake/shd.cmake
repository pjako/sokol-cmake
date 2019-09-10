if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(TOOLS_DIR "${PROJECT_SOURCE_DIR}/tools/win32")    
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    set(TOOLS_DIR "${PROJECT_SOURCE_DIR}/tools/osx")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set(TOOLS_DIR "${PROJECT_SOURCE_DIR}/tools/linux")
endif()

find_program(SHD_BIN_PATH
            NAMES sokol-shdc
            PATHS ${TOOLS_DIR} ENV PATH
            DOC "path to glslcc binary")

function(shd_shader target_name source_files)
    message(STATUS "Found shd: ${SHD_BIN_PATH}")

    foreach(source_file ${source_files})
        set (shd_output_file "${source_file}.h")
        add_custom_command(
            OUTPUT ${shd_output_file}
            COMMAND ${SHD_BIN_PATH}
            ARGS --input ${source_file} --slang ${slang} --output ${shd_output_file}
            DEPENDS ${source_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Compiling ${slang}: ${shd_output_file}"
            VERBATIM
        )

        # add to source files
        
    if(APPLE)
        target_sources(${target_name} PRIVATE ${shd_output_file})
    else()
        target_sources(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${shd_output_file}" "${CMAKE_CURRENT_SOURCE_DIR}/${source_file}")
    endif()
    endforeach()
endfunction()