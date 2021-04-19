# ===========================================================================
# CMake configuration
# ===========================================================================

CMAKE_MINIMUM_REQUIRED (VERSION 3.0)

# ===========================================================================
# Project configuration
# ===========================================================================

PROJECT (antaresXpansion VERSION 0.2.0)

# ===========================================================================
# Default parameters
# ===========================================================================

set(DEFAULT_BUILD_TYPE Release)

# Set Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
endif()

option (USE_MPI "Enable MPI" OFF)
option (USE_SEQUENTIAL "Enable sequential" ON)

IF ((NOT USE_MPI) AND (NOT USE_SEQUENTIAL))
	message (FATAL_ERROR "Need to specify at least an executable target : set USE_MPI to true or USE_SEQUENTIAL to false")
ENDIF ()

#set(ANTARES_XPANSION_RC 3)

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
	include(SetUpCoverage)
	SetUpCoverage()
endif ()

# ===========================================================================
# External dependencies
# ===========================================================================
#CMake function
include("cmake/utils.cmake")
include("cmake/json-cmake-1.1.0/JSONParser.cmake")

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

# Add DEPS_INSTALL_DIR with antares-simulator CMAKE_BUILD_TYPE special case
if (${CMAKE_BUILD_TYPE} STREQUAL  "Release")

	string(REPLACE ${CMAKE_BUILD_TYPE} release
			DEPS_INSTALL_DIR_ANTARES
			${DEPS_INSTALL_DIR})
	
	list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})
	
elseif(${CMAKE_BUILD_TYPE} STREQUAL  "Debug")

	string(REPLACE ${CMAKE_BUILD_TYPE} debug
			DEPS_INSTALL_DIR_ANTARES
			${DEPS_INSTALL_DIR})
	
	list(APPEND CMAKE_PREFIX_PATH ${DEPS_INSTALL_DIR_ANTARES})

endif()

message(STATUS "CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")

file(READ antares-version.json jsonContent)
sbeParseJson(antares_version jsonContent)

#need to know antares solver version because antares-solver targets still refers to antares version
SET(ANTARES_VERSION ${antares_version.antares_version_executable})
SET(ANTARES_VERSION_TAG ${antares_version.antares_version})

sbeClearJson(antares_version)

add_subdirectory(antares-deps)

# Build dependencies not handled by antares-deps repository
option(BUILD_antares_solver "Build antares solver" ON)
message(STATUS "Build antares solver: ${BUILD_antares_solver}")

add_subdirectory(cmake/dependencies ${CMAKE_CURRENT_BINARY_DIR}/build_deps)

# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------

find_package (Boost REQUIRED COMPONENTS program_options)
	
	
if (USE_MPI)
	# ugly fix required due to conan packages and policies vs. boost brainfucking name rules
	if (MSVC)
		list (APPEND CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
	endif ()
	find_package(MPI REQUIRED)
	find_package (Boost REQUIRED COMPONENTS mpi serialization)
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

if(WIN32)
	set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
	set(CURRENT_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package(antares-solver REQUIRED)

if (${CMAKE_BUILD_TYPE} STREQUAL  "Release")
get_target_property( ANTARES_SOLVER_PATH antares-${ANTARES_VERSION}-solver IMPORTED_LOCATION_RELEASE )
else()
get_target_property( ANTARES_SOLVER_PATH antares-${ANTARES_VERSION}-solver IMPORTED_LOCATION_DEBUG )
endif()
get_filename_component(ANTARES_SOLVER_DIR ${ANTARES_SOLVER_PATH} DIRECTORY)
get_filename_component(ANTARES_SOLVER_APP ${ANTARES_SOLVER_PATH} NAME)

#Copy antares-solver to build directory
FILE(COPY ${ANTARES_SOLVER_PATH}
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
    USE_SOURCE_PERMISSIONS)  
    
#Copy associated .so and .dll to build directory
FILE(COPY  ${ANTARES_SOLVER_DIR}/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.dll")

FILE(COPY ${ANTARES_SOLVER_DIR}/../lib/
	DESTINATION ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}
	USE_SOURCE_PERMISSIONS
	FILES_MATCHING PATTERN "*.so")

add_subdirectory ("${PROJECT_SOURCE_DIR}/src")

if(BUILD_TESTING)
	enable_testing()
	add_subdirectory("${PROJECT_SOURCE_DIR}/tests")
endif()


# ===========================================================================
# Documentation
# ===========================================================================

add_subdirectory ("${PROJECT_SOURCE_DIR}/documentation")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/documentation/antares-xpansion-user-guide-0.2.0.pdf
        DESTINATION documentation)
        
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/" 		DESTINATION examples)

if (WIN32)
  install(DIRECTORY ${CURRENT_RUNTIME_OUTPUT_DIRECTORY}/
    TYPE BIN
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.dll")
endif()

install(DIRECTORY ${ANTARES_SOLVER_DIR}/
  TYPE BIN
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "*.dll")
  
install(DIRECTORY ${ANTARES_SOLVER_DIR}/../lib/
  TYPE BIN
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING PATTERN "*.so")

install(PROGRAMS ${ANTARES_SOLVER_PATH}
  TYPE BIN)
  
find_package(antares-study-updater)
if (antares-study-updater_FOUND)

        if (${CMAKE_BUILD_TYPE} STREQUAL  "Release")
	    get_target_property( ANTARES_STUDY_UPDATER_PATH antares-${ANTARES_VERSION}-study-updater IMPORTED_LOCATION_RELEASE )
	else()	
	    get_target_property( ANTARES_STUDY_UPDATER_PATH antares-${ANTARES_VERSION}-study-updater IMPORTED_LOCATION_DEBUG )
	endif()

	install(PROGRAMS ${ANTARES_STUDY_UPDATER_PATH}
  		TYPE BIN)
      
endif()

include (InstallRequiredSystemLibraries)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    get_linux_lsb_release_information()
    message(STATUS "Linux ${LSB_RELEASE_ID_SHORT} ${LSB_RELEASE_VERSION_SHORT} ${LSB_RELEASE_CODENAME_SHORT}")
    set(CPACK_SYSTEM_NAME "${LSB_RELEASE_ID_SHORT}-${LSB_RELEASE_VERSION_SHORT}")
endif()

set(CPACK_PROJECT_NAME "antaresXpansion")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 2)
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "RTE")
set(CPACK_DEBIAN_PACKAGE_NAME "antares-xpansion")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4,libjsoncpp1,libboost-mpi-dev")
set(CPACK_RPM_PACKAGE_REQUIRES "openmpi,jsoncpp,boost-openmpi")
set(CPACK_RPM_PACKAGE_AUTOREQPROV "0")

if (ANTARES_XPANSION_RC)
	set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-rc${ANTARES_XPANSION_RC}")
endif()

include(CPack)

