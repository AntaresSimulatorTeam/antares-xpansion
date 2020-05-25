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

# ===========================================================================
# Compiler options
# ===========================================================================
SET(EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
SET(   LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

set (CMAKE_CXX_STANDARD 11)

IF( USE_MPI )
	SET(CMAKE_CXX_COMPILER mpicxx)
ENDIF( USE_MPI )
MESSAGE("USE_MPI is ${USE_MPI}")

IF( WIN32 )
#	SET( CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE STRING "limited configs" FORCE)
#	SET( CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1" )
#	SET( CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
ELSE()
	SET ( CMAKE_BUILD_TYPE "RELEASE"     )
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
ENDIF( WIN32 )

MESSAGE("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
MESSAGE("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

MESSAGE("XPRESSDIR is $ENV{XPRESSDIR}")
MESSAGE("BOOSTDIR  is $ENV{BOOSTDIR}")


# ===========================================================================
# Include directories
# ===========================================================================
INCLUDE_DIRECTORIES(
	./src_cpp
	$ENV{XPRESSDIR}/include
	)

IF( USE_MPI )
	IF(WIN32)
		INCLUDE_DIRECTORIES($ENV{BOOSTDIR} ./src_mpi $ENV{MSMPI_INC} $ENV{MSMPI_INC}/x64)
		LINK_DIRECTORIES($ENV{XPRESSDIR}/lib $ENV{BOOSTDIR}/stage/lib $ENV{MSMPI_LIB64})
	ELSE(WIN32)
		INCLUDE_DIRECTORIES($ENV{BOOSTDIR} ./src_mpi)
		LINK_DIRECTORIES($ENV{XPRESSDIR}/lib $ENV{BOOSTDIR}/stage/lib)
	ENDIF(WIN32)
ENDIF( USE_MPI )


# ===========================================================================
# Link directories
# ===========================================================================
LINK_DIRECTORIES($ENV{XPRESSDIR}/lib)

# ===========================================================================
# Targets
# ===========================================================================
add_subdirectory ("${PROJECT_SOURCE_DIR}/src_cpp")

IF( USE_MPI )
	add_subdirectory ("${PROJECT_SOURCE_DIR}/src_mpi")
ENDIF( USE_MPI )

add_subdirectory ("${PROJECT_SOURCE_DIR}/src_enzo_test")

add_subdirectory ("${PROJECT_SOURCE_DIR}/exe")