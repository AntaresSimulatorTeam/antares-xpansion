# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (benders_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/SubproblemWorker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/SimulationOptions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersBase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersStructsDatas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Worker.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LastIterationWriter.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/LastIterationReader.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/LastIterationPrinter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StartUp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersMathLogger.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/MasterUpdateBase.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/CutsManagement.cpp

	)


target_include_directories (benders_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries (benders_core
	PUBLIC
		helpers
		antaresXpansion::output_core
		solvers
		${JSONCPP_LIB}
		outer_loop_lib
)
