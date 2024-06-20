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

# build_git_dependency()
#
# CMake function to download, build and install (in staging area) a dependency at configure
# time.
#
# Parameters:
# NAME: name of the dependency
# REPOSITORY: git url of the dependency
# TAG: tag of the dependency
# APPLY_PATCH: apply patch
# CMAKE_ARGS: List of specific CMake args to add
# CONFIGURE_COMMAND : Command used for configure (default empty and default CMake configure use)
# BUILD_COMMAND : Command used for build (default empty and default CMake build use)
# INSTALL_COMMAND : Command used for install (default empty and default CMake instal use)
# INSOURCE_BUILD : If option ON , compilation is done in source directory
# SOURCE_SUBDIR : Indicate where to find CMakeList.txt
#
# build_dependency(
#   NAME
#     abseil-cpp
#   URL
#     https://github.com/abseil/abseil-cpp.git
#   TAG
#     master
#   APPLY_PATCH
#     ${CMAKE_SOURCE_DIR}/patches/abseil-cpp.patch
# )
function(build_git_dependency)
    set(options "")
    set(oneValueArgs NAME REPOSITORY TAG APPLY_PATCH CONFIGURE_COMMAND BUILD_COMMAND INSTALL_COMMAND INSOURCE_BUILD SOURCE_SUBDIR)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(GIT_DEP
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )
    message(STATUS "Building ${GIT_DEP_NAME}: ...")

    if(GIT_DEP_APPLY_PATCH)
        set(PATCH_CMD "git apply \"${GIT_DEP_APPLY_PATCH}\"")
    else()
        set(PATCH_CMD "\"\"")
    endif()

    if(GIT_DEP_CONFIGURE_COMMAND)
        set(CONFIGURE_COMMAND "CONFIGURE_COMMAND ${GIT_DEP_CONFIGURE_COMMAND}")
    else()
        set(CONFIGURE_COMMAND "#CONFIGURE_COMMAND")
    endif()

    if(GIT_DEP_BUILD_COMMAND)
        set(BUILD_COMMAND "BUILD_COMMAND ${GIT_DEP_BUILD_COMMAND}")
    else()
        set(BUILD_COMMAND "#BUILD_COMMAND")
    endif()

    if(GIT_DEP_INSTALL_COMMAND)
        set(INSTALL_COMMAND "INSTALL_COMMAND ${GIT_DEP_INSTALL_COMMAND}")
    else()
        set(INSTALL_COMMAND "#INSTALL_COMMAND")
    endif()

    if(GIT_DEP_SOURCE_SUBDIR)
        set(SOURCE_SUBDIR "SOURCE_SUBDIR ${GIT_DEP_SOURCE_SUBDIR}")
    else()
        set(SOURCE_SUBDIR "#SOURCE_SUBDIR")
    endif()

    if (GIT_DEP_INSOURCE_BUILD)
        set (BINARY_DIR "BUILD_IN_SOURCE 1")
    else()
        set (BINARY_DIR "BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${GIT_DEP_NAME}/build")
    endif()

    #use specific list separator to be able to use a list of CMAKE_PREFIX_PATH
    string(REPLACE ";" "|" STR_CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")

    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in
            ${CMAKE_CURRENT_BINARY_DIR}/${GIT_DEP_NAME}/CMakeLists.txt @ONLY)

    execute_process(
            COMMAND ${CMAKE_COMMAND} -H. -Bproject_build -G "${CMAKE_GENERATOR}"
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${GIT_DEP_NAME})
    if(result)
        message(FATAL_ERROR "CMake step for ${GIT_DEP_NAME} failed: ${result}")
    endif()

    include(ProcessorCount)
    ProcessorCount(NB_PROC)

    execute_process(
            COMMAND ${CMAKE_COMMAND} --build project_build --config ${CMAKE_BUILD_TYPE} -j ${NB_PROC}
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${GIT_DEP_NAME})
    if(result)
        message(FATAL_ERROR "Build step for ${GIT_DEP_NAME} failed: ${result}")
    endif()

    message(STATUS "Building ${GIT_DEP_NAME}: ...DONE")
endfunction()
