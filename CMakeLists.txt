# ===========================================================================
# CMake configuration
# ===========================================================================

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# ===========================================================================
# Project configuration
# ===========================================================================

PROJECT(antaresXpansion VERSION 1.3.2)
set(ANTARES_XPANSION_RC 0)

# ===========================================================================
# Default parameters
# ===========================================================================
if (MSVC)
    cmake_policy(SET CMP0111 OLD) #To remove warning for not found configuration on MSVC
endif ()

if (CMAKE_TOOLCHAIN_FILE)
    if (NOT IS_ABSOLUTE ${CMAKE_TOOLCHAIN_FILE})
        SET(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_TOOLCHAIN_FILE})
    endif ()
endif ()

set(DEFAULT_BUILD_TYPE Release)

# Set Build Type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
endif ()

# for centos docker to run MPI tests as root
if (ALLOW_RUN_AS_ROOT)
	set(ALLOW_RUN_AS_ROOT "true")
else ()
	set(ALLOW_RUN_AS_ROOT "false")
endif ()

# Set solvers to use
option(COIN_OR "Use COIN Clp and Cbc solvers" ON)
option(XPRESS "Use solver FICO XPRESS" OFF)

if (COIN_OR)
    message("Coin-OR Solvers Clp and Cbc used. Solvers version are those present in orTools release linked to project.")
else ()
	message(FATAL_ERROR "COIN_OR has to be true, it is required in lpnamer module of Antares Xpansion.")
endif()


# ===========================================================================
# CMAKE ADDITIONAL MODULES
# ===========================================================================

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


set(CMAKE_INSTALL_RPATH $ORIGIN)

# ===========================================================================
# Compiler options
# ===========================================================================

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    message("WINDOWS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(RUNTIME_IGNORE_LIST_RELEASE "/NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib /NODEFAULTLIB:libcmtd.lib")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE " ${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE " ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE " ${CMAKE_STATIC_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /EHsc")


else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -fmessage-length=0 -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
endif ()

message("cmake_build_type : ${CMAKE_BUILD_TYPE}")
message("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
message("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

if (CODE_COVERAGE)
    # if code-coverage is ON, force tests build
    set(BUILD_TESTING ON)

    include(CodeCoverage)
    code_coverage(NAME code-coverage
            OUTPUT_DIR coverage
            EXCLUDE_DIRS tests)
endif ()

# ===========================================================================
# External dependencies
# ===========================================================================
#CMake function
include("cmake/utils.cmake")
include("cmake/json-cmake-1.1.0/JSONParser.cmake")

file(READ antares-version.json jsonContent)
sbeParseJson(antares_version jsonContent)

#need to know antares solver version because antares-solver targets still refers to antares version
SET(ANTARES_VERSION ${antares_version.antares_version_executable})
SET(ANTARES_VERSION_TAG ${antares_version.antares_version})
SET(MINIZIP_TAG ${antares_version.minizip_ng_version})

sbeClearJson(antares_version)

# Build dependencies not handled by antares-deps repository
option(BUILD_antares_solver "Build antares solver" ON)
message(STATUS "Build antares solver: ${BUILD_antares_solver}")
if (${BUILD_antares_solver})
    #check if antares_solver available before asking for sirius and ortools build
    find_package(Antares QUIET)
    if (NOT Antares_FOUND)
        message(WARNING "Antares solver not found, will be build")
        set(BUILD_sirius ON)
        set(BUILD_ortools ON)
        if (NOT Antares_FOUND)
            message(STATUS "Antares not found, building it from source")
            #Define deps install directory
            if (NOT DEPS_INSTALL_DIR)
                SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rte-antares-deps-${CMAKE_BUILD_TYPE})
            else ()
                if (NOT IS_ABSOLUTE ${DEPS_INSTALL_DIR})
                    SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${DEPS_INSTALL_DIR})
                endif ()
            endif ()

            # Build Needed dependencies
            list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR})

            # Add DEPS_INSTALL_DIR with antares-simulator CMAKE_BUILD_TYPE special case
            if (${CMAKE_BUILD_TYPE} STREQUAL "Release")

                string(REPLACE ${CMAKE_BUILD_TYPE} release
                        DEPS_INSTALL_DIR_ANTARES
                        ${DEPS_INSTALL_DIR})

                list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})

            elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

                string(REPLACE ${CMAKE_BUILD_TYPE} debug
                        DEPS_INSTALL_DIR_ANTARES
                        ${DEPS_INSTALL_DIR})

                list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})

            endif ()
            add_subdirectory(cmake/dependencies ${CMAKE_CURRENT_BINARY_DIR}/build_deps)
        endif ()
    endif ()
endif ()
find_package(Antares REQUIRED)
message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
set(CMAKE_FIND_DEBUG_MODE false)

find_package(minizip-ng REQUIRED)
# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------

find_package(Boost REQUIRED COMPONENTS program_options)

# ugly fix required due to conan packages and policies vs. boost brainfucking name rules
if (MSVC)
    list(APPEND CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
endif ()
find_package(MPI REQUIRED)

find_package(Boost REQUIRED COMPONENTS mpi serialization)
include_directories(${Boost_INCLUDE_DIRS})

message(STATUS "BOOST_MPI_LIBRARY : " ${Boost_MPI_LIBRARY})
message(STATUS "Boost_SERIALIZATION_LIBRARY : " ${Boost_SERIALIZATION_LIBRARY})
message(STATUS "BOOST_LIBRARIES : " ${Boost_INCLUDE_DIRS})

install(FILES
        $<TARGET_FILE:Boost::mpi>
        $<TARGET_FILE:Boost::serialization>
        $<TARGET_FILE:Boost::program_options>
        DESTINATION bin)

# ---------------------------------------------------------------------------
# Threads
# ---------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif ()
find_package(Threads REQUIRED)

find_package(ZLIB REQUIRED) #Required for CoinUtils

set(GFLAGS_USE_TARGET_NAMESPACE TRUE)
find_package(gflags REQUIRED)
find_package(Cbc REQUIRED)
find_package(Cgl REQUIRED)
find_package(Clp REQUIRED)
find_package(CoinUtils REQUIRED)
find_package(Osi REQUIRED)

if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif ()

if (SOLVER)
    if (SOLVER STREQUAL "xpress")
        add_definitions(-DXPRESS_SOLVER -DUSE_XPRESS)
    elseif (SOLVER STREQUAL "coin")#already achieved by default through ORTOOLS_DEFINITIONS
        add_definitions(-DUSE_CBC -DUSE_CLP)
    else ()
        message(FATAL_ERROR "unsupported value, ${SOLVER}, for solver option.")
    endif ()
endif ()

find_package(jsoncpp CONFIG REQUIRED)
if (MSVC)
    set(JSONCPP_LIB jsoncpp_lib)
else ()
    set(JSONCPP_LIB jsoncpp_static)
endif ()
message("Using jsoncpp target ${JSONCPP_LIB}")

install(FILES
        $<TARGET_FILE:${JSONCPP_LIB}>
        DESTINATION bin
)

## Coin-OR (Clp and CBC solvers)
if (COIN_OR)
    message("COIN-OR is ${COIN_OR}")
    # Add solver variables for usage in C++ source code
    add_definitions(-DCOIN_OR=true)
endif (COIN_OR)


# ===========================================================================
# Targets
# ===========================================================================
option(BUILD_UI "Build antares-xpansion ui" OFF)
option(BUILD_TESTING "Activates unit tests building" OFF)

if (WIN32)
    set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else ()
    set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif ()

find_package(antares-solver REQUIRED)

#Default RELEASE antares-solver use
get_target_property(ANTARES_SOLVER_PATH antares-solver IMPORTED_LOCATION_RELEASE)
if (NOT ANTARES_SOLVER_PATH)
    get_target_property(ANTARES_SOLVER_PATH antares-solver IMPORTED_LOCATION_DEBUG)
endif ()

get_filename_component(ANTARES_SOLVER_DIR ${ANTARES_SOLVER_PATH} DIRECTORY)
get_filename_component(ANTARES_SOLVER_APP ${ANTARES_SOLVER_PATH} NAME)

#Copy antares-solver to build directory
FILE(COPY ${ANTARES_SOLVER_PATH}
        DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
        USE_SOURCE_PERMISSIONS)

#Copy associated .so and .dll to build directory
FILE(COPY ${ANTARES_SOLVER_DIR}/
        DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.dll")

FILE(COPY ${ANTARES_SOLVER_DIR}/
        DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.so")

#Glob *.so files in antares_solver_dir
file(GLOB_RECURSE ANTARES_SOLVER_LIBS ${ANTARES_SOLVER_DIR}/*.so)
INSTALL(FILES ${ANTARES_SOLVER_LIBS} DESTINATION bin)
file(GLOB_RECURSE ANTARES_SOLVER_LIBS ${ANTARES_SOLVER_DIR}/*.dll)
INSTALL(FILES ${ANTARES_SOLVER_LIBS} DESTINATION bin)


FILE(COPY ${ANTARES_SOLVER_DIR}/../lib/
        DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.so")

FILE(COPY ${ANTARES_SOLVER_DIR}/../bin/
        DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.so")

add_subdirectory("${PROJECT_SOURCE_DIR}/src")

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif ()


# ===========================================================================
# Documentation
# ===========================================================================

if (USER_GUIDE_PATH)
    install(FILES ${USER_GUIDE_PATH} DESTINATION .)
endif ()

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/" DESTINATION examples)
if (WIN32)
    cmake_path(GET MPIEXEC_EXECUTABLE PARENT_PATH MPI_BIN_DIR)

    #Copy associated .so and .dll to build directory
    FILE(COPY ${MPI_BIN_DIR}/
            DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
            USE_SOURCE_PERMISSIONS)
    find_file(msmpi_dll NAMES msmpi.dll REGISTRY_VIEW HOST)

    install(DIRECTORY ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}/
            TYPE BIN
            USE_SOURCE_PERMISSIONS
            FILES_MATCHING PATTERN "*.dll")

    install(FILES
            ${MPI_msmpi_LIBRARY}
            ${msmpi_dll}
            DESTINATION bin
    )

    install(DIRECTORY ${MPI_BIN_DIR}/
            TYPE BIN)
endif ()

install(DIRECTORY ${ANTARES_SOLVER_DIR}/
        TYPE BIN
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.dll")

install(DIRECTORY ${ANTARES_SOLVER_DIR}/../lib/
        TYPE BIN
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.so")

#Mostly for sirius
install(DIRECTORY ${ANTARES_SOLVER_DIR}/../bin/
        TYPE BIN
        USE_SOURCE_PERMISSIONS
        FILES_MATCHING PATTERN "*.so")

install(PROGRAMS ${ANTARES_SOLVER_PATH}
        TYPE BIN)

include(InstallRequiredSystemLibraries)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
    set(CPACK_SYSTEM_NAME "${LSB_RELEASE_ID_SHORT}-${LSB_RELEASE_VERSION_SHORT}")
endif ()

set(CPACK_PROJECT_NAME "antaresXpansion")


set(CPACK_PACKAGE_NAME "${CPACK_PROJECT_NAME}")

if (XPRESS)
    set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-xpress")
endif (XPRESS)


set(CPACK_DEBIAN_PACKAGE_MAINTAINER "RTE")
set(CPACK_DEBIAN_PACKAGE_NAME "antares-xpansion")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4,libjsoncpp1,libboost-mpi-dev")
set(CPACK_RPM_PACKAGE_REQUIRES "openmpi,jsoncpp,boost-openmpi")
set(CPACK_RPM_PACKAGE_AUTOREQPROV "0")

if (ANTARES_XPANSION_RC)
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-rc${ANTARES_XPANSION_RC}")
endif ()

# Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

#version file to define Xpansion and Antares-simulator versions
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/python/antares_xpansion/__version__.py.in ${CMAKE_CURRENT_SOURCE_DIR}/src/python/antares_xpansion/__version__.py)
include(CPack)

add_custom_target(graphviz
        COMMAND ${CMAKE_COMMAND} "--graphviz=xpansion.dot" .
        COMMAND dot -Tsvg xpansion.dot -o xpansion.svg
        COMMAND dot -Tpng xpansion.dot -o xpansion.png
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
