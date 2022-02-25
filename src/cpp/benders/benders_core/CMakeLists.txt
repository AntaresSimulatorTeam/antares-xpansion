# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (benders_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/SimplexBasis.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerSlave.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SimulationOptions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersBase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SlaveCut.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersStructsDatas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.cpp
        )




target_include_directories (benders_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries (benders_core
	PUBLIC
		helpers
		output_core
		solvers
		glog::glog
)

add_library (${PROJECT_NAME}::benders_core ALIAS benders_core)