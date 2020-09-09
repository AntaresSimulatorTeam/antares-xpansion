# ===========================================================================
# CMake configuration
# ===========================================================================

CMAKE_MINIMUM_REQUIRED (VERSION 3.14)

# ===========================================================================
# Project configuration
# ===========================================================================

PROJECT (antaresXpansion VERSION 1.0.0)

# ===========================================================================
# Default parameters
# ===========================================================================

#Please enable this option if ortools v7.1 or later is used
option (ORTOOLS_RECENT "ORTools is v7.1 or later" ON)
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
find_package (ortools REQUIRED)
add_definitions (${ORTOOLS_DEFINITIONS})
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

#IF (USE_MPI)
#	IF (WIN32)
#		message ("MPI_INC $ENV{MSMPI_INC}")
#		message ("MPI_LIB64 $ENV{MSMPI_LIB64}")
#		INCLUDE_DIRECTORIES ($ENV{MSMPI_INC} $ENV{MSMPI_INC}/x64)
#		LINK_DIRECTORIES ($ENV{MSMPI_LIB64})
#	ENDIF (WIN32)
#ENDIF (USE_MPI)

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${PROJECT_SOURCE_DIR}/src/src_cpp")

# ===========================================================================
# Documentation
# ===========================================================================

add_subdirectory ("${PROJECT_SOURCE_DIR}/documentation")
