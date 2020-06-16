# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)


SET(SEQUENTIAL_FILES

	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/launcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerStabilizedMaster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersFunctions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Benders.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerTrace.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/WorkerStabilizedMaster.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersFunctions.h
	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.h
	${CMAKE_CURRENT_SOURCE_DIR}/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerTrace.h
	${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
	${CMAKE_CURRENT_SOURCE_DIR}/launcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.h
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.h
	${CMAKE_CURRENT_SOURCE_DIR}/Benders.h

	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.hxx
)

ADD_LIBRARY(sequentialcore STATIC ${SEQUENTIAL_FILES} )

target_include_directories(sequentialcore
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
)

target_link_libraries (sequentialcore
	PUBLIC
		ortools
		helpers
)

ADD_LIBRARY (${PROJECT_NAME}::sequentialcore ALIAS sequentialcore)