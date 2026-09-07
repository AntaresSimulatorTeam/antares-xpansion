
add_library(tests_utils STATIC
						RandomDirGenerator.h RandomDirGenerator.cpp
						LogPrefixManip.h LogPrefixManip.cpp NOOPSolver.h EmptyLogManager.h
						 )
target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(tests_utils PUBLIC antaresXpansion::benders_core)
add_library (${PROJECT_NAME}::tests_utils ALIAS tests_utils)
