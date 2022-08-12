macro(copy_dependency deps target)

    if( MSVC )
        if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")

            get_target_property( DEP_SHARED_LIB_PATH ${deps} IMPORTED_LOCATION_RELEASE )
        else()

            get_target_property( DEP_SHARED_LIB_PATH ${deps} IMPORTED_LOCATION_DEBUG )
        endif()

        # Copy the shared lib file
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DEP_SHARED_LIB_PATH} $<TARGET_FILE_DIR:${target}>)
    endif()

endmacro()

function(get_linux_lsb_release_information)
    find_program(LSB_RELEASE_EXEC lsb_release)
    if(NOT LSB_RELEASE_EXEC)
        message(FATAL_ERROR "Could not detect lsb_release executable, can not gather required information")
    endif()

    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --id OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --release OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --codename OUTPUT_VARIABLE LSB_RELEASE_CODENAME_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(LSB_RELEASE_ID_SHORT "${LSB_RELEASE_ID_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_VERSION_SHORT "${LSB_RELEASE_VERSION_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_CODENAME_SHORT "${LSB_RELEASE_CODENAME_SHORT}" PARENT_SCOPE)
endfunction()

function(find_python_module module)
    set(Python3_FIND_REGISTRY "LAST")
    find_package(Python3 COMPONENTS Interpreter)

    if(Python3_Interpreter_FOUND)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import ${module}"
            RESULT_VARIABLE EXIT_CODE
            OUTPUT_QUIET
            ERROR_QUIET
        )
        
        if (${EXIT_CODE} EQUAL 0)
            set(PYTHON_MODULE_${module}_FOUND "true" PARENT_SCOPE)
        endif()
        
    endif()
    
endfunction()