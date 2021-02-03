# ===========================================================================
# CMake configuration
# ===========================================================================

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

# ===========================================================================
# Project configuration
# ===========================================================================

PROJECT (antaresXpansion VERSION 0.1.0)

# ===========================================================================
# Default parameters
# ===========================================================================

option (USE_MPI "Enable MPI" OFF)
option (USE_SEQUENTIAL "Enable sequential" OFF)

IF ((NOT USE_MPI) AND (NOT USE_SEQUENTIAL))
	message (FATAL_ERROR "Need to specify at least an executable target : set USE_MPI to true or USE_SEQUENTIAL to false")
ENDIF ()

set (BOOST_MIN_VERSION "1.65.1")

# ===========================================================================
# CMAKE ADDITIONAL MODULES
# ===========================================================================

LIST (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


set(CMAKE_INSTALL_RPATH 		$ORIGIN)

# ===========================================================================
# Compiler options
# ===========================================================================

set (EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
set (LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (USE_MPI)
	set (CMAKE_CXX_COMPILER mpicxx)
endif ()
message ("USE_MPI is ${USE_MPI}")

if (MSVC)
	message ("WINDOWS")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set (RUNTIME_IGNORE_LIST_RELEASE "/NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib /NODEFAULTLIB:libcmtd.lib")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE " ${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE " ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
	set (CMAKE_STATIC_LINKER_FLAGS_RELEASE " ${CMAKE_STATIC_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
		
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /EHsc")
    
    

else ()
#   set (CMAKE_BUILD_TYPE "RELEASE")
#	set (CMAKE_BUILD_TYPE "DEBUG")
#	IF (WITH_DEBUG)
#		set (CMAKE_BUILD_TYPE "DEBUG")
#	ELSE (WITH_DEBUG)GENDIOR
#		set (CMAKE_BUILD_TYPE "RELEASE")
#	ENDIF (WITH_DEBUG)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -fmessage-length=0 -fPIC")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
endif ()

message ("cmake_build_type : ${CMAKE_BUILD_TYPE}")
message ("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
message ("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
message ("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
message ("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

# ----------------------------------------------------------------------------
# Coverage build type definition
# ----------------------------------------------------------------------------
option (${PROJECT_NAME}_WITH_COVERAGE "enable coverage" OFF)
if (${PROJECT_NAME}_WITH_COVERAGE)
	include (SetUpCoverage)
	SetUpCoverage ()
endif ()

# ===========================================================================
# External dependencies
# ===========================================================================
#CMake function
include("cmake/utils.cmake")

#Define install directory
if (NOT DEPS_INSTALL_DIR)
    SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../rte-antares-deps-${CMAKE_BUILD_TYPE})

else()
    if(NOT IS_ABSOLUTE ${DEPS_INSTALL_DIR})
        SET(DEPS_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${DEPS_INSTALL_DIR})
    endif()    
endif()

# Build Needed dependencies
list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR})
message(STATUS "CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")

add_subdirectory(antares-deps)

# Build dependencies not handled by antares-deps repository
option(BUILD_antares_solver "Build antares solver" OFF)
message(STATUS "Build antares solver: ${BUILD_antares_solver}")

add_subdirectory(cmake/dependencies ${CMAKE_CURRENT_BINARY_DIR}/build_deps)

# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------
if (USE_MPI)
	# ugly fix required due to conan packages and policies vs. boost brainfucking name rules
	if (MSVC)
		list (APPEND CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
	endif ()

	find_package (Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS mpi serialization)
	include_directories (${Boost_INCLUDE_DIRS})

	message (STATUS "BOOST_MPI_LIBRARY : " ${Boost_MPI_LIBRARY})
	message (STATUS "Boost_SERIALIZATION_LIBRARY : " ${Boost_SERIALIZATION_LIBRARY})
	message (STATUS "BOOST_LIBRARIES : " ${Boost_INCLUDE_DIRS})
endif ()

# ---------------------------------------------------------------------------
# Threads
# ---------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
	set (THREADS_PREFER_PTHREAD_FLAG ON)
endif ()
find_package (Threads REQUIRED)

# OR-Tools
#gflags needed for ortools
set(GFLAGS_USE_TARGET_NAMESPACE TRUE) 
find_package(gflags)

find_package(ortools)
if(NOT ortools_FOUND)
	message (FATAL_ERROR "OR-Tools not found. OR-Tools can be compiled with -DBUILD_ortools=ON or you can specify previous dependency install directory with -DCMAKE_PREFIX_PATH or -DDEPS_INSTALL_DIR")
endif()

if(POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW)
endif()

find_package(sirius_solver REQUIRED)

if (SOLVER)
	if (SOLVER STREQUAL "cplex")
		add_definitions (-DCPLEX_SOLVER -DUSE_CPLEX)
	elseif (SOLVER STREQUAL "gurobi")
		add_definitions (-DGUROBI_SOLVER -DUSE_GUROBI)
	elseif (SOLVER STREQUAL "xpress")
		add_definitions (-DXPRESS_SOLVER -DUSE_XPRESS)
	elseif (SOLVER STREQUAL "coin")#already achieved by default through ORTOOLS_DEFINITIONS
		add_definitions (-DUSE_CBC -DUSE_CLP)
	else ()
		message (FATAL_ERROR "unsupported value, ${SOLVER}, for solver option.")
	endif ()
endif ()

find_package(jsoncpp CONFIG REQUIRED)

# ===========================================================================
# Targets
# ===========================================================================
option(BUILD_TESTING "Activates unit tests building" OFF)
if(BUILD_TESTING)
	enable_testing()
endif()

if(WIN32)
	set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
	set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

#Copy antares-solver from DEPS_INSTALL_DIR to build directory
FILE(COPY  ${DEPS_INSTALL_DIR}/bin/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "antares-*.*-solver")
    
FILE(COPY  ${DEPS_INSTALL_DIR}/bin/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "antares-*.*-solver.exe")
    
FILE(COPY  ${DEPS_INSTALL_DIR}/bin/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.dll")

FILE(COPY ${DEPS_INSTALL_DIR}/bin/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
	USE_SOURCE_PERMISSIONS
	FILES_MATCHING PATTERN "*.so")

add_subdirectory ("${PROJECT_SOURCE_DIR}/src")

# ===========================================================================
# Documentation
# ===========================================================================

add_subdirectory ("${PROJECT_SOURCE_DIR}/documentation")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/documentation/antares-xpansion-user-guide-0.1.0.pdf
        DESTINATION documentation)
        
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/" 		DESTINATION examples)

if (WIN32)
  install(DIRECTORY ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}/
    TYPE BIN
    FILES_MATCHING PATTERN "*.dll")
endif()

install(DIRECTORY ${DEPS_INSTALL_DIR}/bin/
  TYPE BIN
  FILES_MATCHING PATTERN "*.dll")
  
install(DIRECTORY ${DEPS_INSTALL_DIR}/lib/
  TYPE BIN
  FILES_MATCHING PATTERN "*.so")
  
install(DIRECTORY ${DEPS_INSTALL_DIR}/bin/
  TYPE BIN
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "antares-*.*-solver.exe")
  
install(DIRECTORY ${DEPS_INSTALL_DIR}/bin/
  TYPE BIN
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "antares-*.*-solver")
  
  
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
    set(CPACK_SYSTEM_NAME "${LSB_RELEASE_ID_SHORT}-${LSB_RELEASE_VERSION_SHORT}") 
endif()
  
#For now only test archive, need to define several CPACK variable
include(CPack)
set(CPACK_PROJECT_NAME "antaresXpansion")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
