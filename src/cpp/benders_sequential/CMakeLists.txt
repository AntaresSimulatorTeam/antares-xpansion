# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (benders_sequential_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.h
	${CMAKE_CURRENT_SOURCE_DIR}/launcher.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerStabilizedMaster.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.h
	${CMAKE_CURRENT_SOURCE_DIR}/BendersOptions.hxx
	${CMAKE_CURRENT_SOURCE_DIR}/BendersFunctions.h
	${CMAKE_CURRENT_SOURCE_DIR}/Benders.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.h
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.h
	${CMAKE_CURRENT_SOURCE_DIR}/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerTrace.h
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.h
	${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.h
	${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
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
	${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.cpp
)

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories (benders_sequential_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
                ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (benders_sequential_core
	PUBLIC
		helpers
		jsoncpp_lib
)

add_library (${PROJECT_NAME}::benders_sequential_core ALIAS benders_sequential_core)
