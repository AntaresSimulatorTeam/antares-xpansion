CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
PROJECT(antaresXpansion VERSION 0.0.0)

SET(EXECUTABLE_OUTPUT_PATH	${CMAKE_BINARY_DIR})
SET(   LIBRARY_OUTPUT_PATH	${CMAKE_BINARY_DIR})

SET(USE_MPI FALSE)

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
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -fmessage-length=0 -fPIC" )
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-conversion -Wno-sign-compare")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing -Wno-unused-parameter")
ENDIF( WIN32 )

MESSAGE("CMAKE_BINARY_DIR : ${CMAKE_BINARY_DIR}")
MESSAGE("CMAKE_SOURCE_DIR : ${CMAKE_SOURCE_DIR}")
MESSAGE("CMAKE_C_COMPILER_VERSION : ${CMAKE_C_COMPILER_VERSION}")
MESSAGE("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")

MESSAGE("XPRESSDIR is $ENV{XPRESSDIR}")
MESSAGE("BOOSTDIR  is $ENV{BOOSTDIR}")
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

LINK_DIRECTORIES($ENV{XPRESSDIR}/lib)


FILE(
	GLOB
	SEQUENTIAL_FILES
	./src_cpp/*.cc
	./src_cpp/*.cpp
	./src_cpp/*.h  
	./src_cpp/*.hxx
)
ADD_LIBRARY(sequentialcore STATIC ${SEQUENTIAL_FILES} )
IF( USE_MPI )
	FILE(
		GLOB
		MPI_FILES
		./src_mpi/*.cc
		./src_mpi/*.cpp
		./src_mpi/*.h  
		./src_mpi/*.hxx
	)
	ADD_LIBRARY(mpicore STATIC ${MPI_FILES} )
ENDIF( USE_MPI )

ADD_EXECUTABLE(test_sur_les_set ./src_enzo_test/test_sur_les_set.cpp)
TARGET_LINK_LIBRARIES(test_sur_les_set sequentialcore)

ADD_EXECUTABLE(merge_mps ./exe_cpp/merge_mps.cpp)
TARGET_LINK_LIBRARIES(merge_mps sequentialcore xprs)

ADD_EXECUTABLE(analyse_mps ./exe_cpp/analyse_mps.cpp)
TARGET_LINK_LIBRARIES(analyse_mps sequentialcore xprs)

IF( USE_MPI )
	ADD_EXECUTABLE(bendersmpi  exe_mpi/main.cpp)
	IF(WIN32)
		TARGET_LINK_LIBRARIES(bendersmpi sequentialcore mpicore xprs msmpi libboost_serialization-vc141-mt-x64-1_67)
	ELSE(WIN32)
		TARGET_LINK_LIBRARIES(bendersmpi sequentialcore mpicore xprs boost_serialization boost_mpi-mt.so)
	ENDIF(WIN32)
	
ELSE( USE_MPI )
	ADD_EXECUTABLE(benderssequential exe_cpp/main.cpp)
	TARGET_LINK_LIBRARIES(benderssequential sequentialcore xprs )
ENDIF( USE_MPI )
