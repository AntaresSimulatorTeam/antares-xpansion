# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

SET(EXE_MPI_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

ADD_EXECUTABLE(bendersmpi  ${EXE_MPI_FILES})
IF(WIN32)
	TARGET_LINK_LIBRARIES(bendersmpi ${PROJECT_NAME}::sequentialcore ${PROJECT_NAME}::mpicore xprs msmpi libboost_serialization-vc141-mt-x64-1_67)
ELSE(WIN32)
	TARGET_LINK_LIBRARIES(bendersmpi ${PROJECT_NAME}::sequentialcore ${PROJECT_NAME}::mpicore xprs boost_serialization boost_mpi-mt.so)
ENDIF(WIN32)
