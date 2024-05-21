# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package(TBB REQUIRED CONFIG)
if (TBB_VERSION_MAJOR VERSION_LESS "2018")
	message(FATAL_ERROR "Require tbb 2018 or higher.")
endif()
if (TBB_VERSION_MAJOR VERSION_GREATER "2020")
	message(FATAL_ERROR "Require tbb 2018 to 2020.")
endif()

add_library (outer_loop_lib STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/OuterLoop.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MasterUpdateBase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CutsManagement.cpp
	)




target_include_directories (outer_loop_lib
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		)

target_link_libraries (outer_loop_lib
	PUBLIC
		helpers
		benders_core
		benders_mpi_core
		benders_by_batch_core		
)

add_library (${PROJECT_NAME}::outer_loop_lib ALIAS outer_loop_lib)
