# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# ===========================================================================
# Project configuration
# ===========================================================================
PROJECT(antaresXpansion VERSION 0.0.0)

# ===========================================================================
# Default parameters
# ===========================================================================
IF(NOT USE_MPI)
	SET(USE_MPI FALSE)
ENDIF()

IF(NOT NO_SEQUENTIAL)
	SET(NO_SEQUENTIAL FALSE)
ENDIF()

IF((NOT USE_MPI) AND (NO_SEQUENTIAL))
	MESSAGE(FATAL_ERROR "Need to specify at least an executable target : set USE_MPI to true or NO_SEQUENTIAL to false")
ENDIF()

set (BOOST_MIN_VERSION "1.65.1")


# ===========================================================================
# CMAKE ADDITIONAL MODULES
# ===========================================================================

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ===========================================================================
# Compiler options
# ===========================================================================

SET(EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
SET(   LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
IF( USE_MPI )
	SET(CMAKE_CXX_COMPILER mpicxx)
ENDIF( USE_MPI )
MESSAGE("USE_MPI is ${USE_MPI}")

IF( MSVC )
	MESSAGE("WINDOWS")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	set (RUNTIME_IGNORE_LIST_RELEASE "/NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcprtd.lib /NODEFAULTLIB:libcmtd.lib")
	set (CMAKE_EXE_LINKER_FLAGS_RELEASE " ${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
	set (CMAKE_SHARED_LINKER_FLAGS_RELEASE " ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")
	set (CMAKE_STATIC_LINKER_FLAGS_RELEASE " ${CMAKE_STATIC_LINKER_FLAGS_RELEASE} ${RUNTIME_IGNORE_LIST_RELEASE}")

ELSE()
#   SET ( CMAKE_BUILD_TYPE "RELEASE"     )
#	SET ( CMAKE_BUILD_TYPE "DEBUG"     )
#	IF( WITH_DEBUG )
#		SET ( CMAKE_BUILD_TYPE "DEBUG"     )
#	ELSE( WITH_DEBUG )GENDIOR
#		SET ( CMAKE_BUILD_TYPE "RELEASE"     )
#	ENDIF( WITH_DEBUG )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DNDEBUG")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -fmessage-length=0 -fPIC" )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
ENDIF( MSVC )

MESSAGE("cmake_build_type : ${CMAKE_BUILD_TYPE}")
MESSAGE("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
MESSAGE("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

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

# optional conan setup
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	include (${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup ()
elseif (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
	include (${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif ()

# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------
IF(USE_MPI)
	find_package (Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS mpi serialization)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

	message(STATUS "BOOST_MPI_LIBRARY : " ${Boost_MPI_LIBRARY} )
	message(STATUS "Boost_SERIALIZATION_LIBRARY : " ${Boost_SERIALIZATION_LIBRARY} )
	message(STATUS "BOOST_LIBRARIES : " ${Boost_INCLUDE_DIRS} )
ENDIF()

# ---------------------------------------------------------------------------
# Threads
# ---------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
	set (THREADS_PREFER_PTHREAD_FLAG ON)
endif ()
find_package (Threads REQUIRED)


# OR-Tools
find_package(ortools REQUIRED)
add_definitions(${ORTOOLS_DEFINITIONS})
if (NOT MSVC)
	add_definitions(-DUSE_CPLEX)
endif()
# add_definitions(-DCPLEX_SOLVER)


IF( USE_MPI )
	IF(WIN32)
		INCLUDE_DIRECTORIES($ENV{MSMPI_INC} $ENV{MSMPI_INC}/x64)
		LINK_DIRECTORIES($ENV{MSMPI_LIB64})
	ENDIF( WIN32 )
ENDIF( USE_MPI )

# ===========================================================================
# Targets
# ===========================================================================
add_subdirectory ("${PROJECT_SOURCE_DIR}/src/src_cpp")

# ===========================================================================
# Documentation
# ===========================================================================
add_subdirectory ("${PROJECT_SOURCE_DIR}/documentation")
