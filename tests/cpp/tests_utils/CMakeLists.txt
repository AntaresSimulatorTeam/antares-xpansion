
add_library(tests_utils STATIC RandomDirGenerator.h RandomDirGenerator.cpp)
target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
add_library (${PROJECT_NAME}::tests_utils ALIAS tests_utils)
