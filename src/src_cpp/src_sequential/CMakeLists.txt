# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (sequentialcore STATIC 
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


target_include_directories (sequentialcore
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
                ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (sequentialcore
	PUBLIC
		helpers
		jsoncpp_lib
)

add_library (${PROJECT_NAME}::sequentialcore ALIAS sequentialcore)
