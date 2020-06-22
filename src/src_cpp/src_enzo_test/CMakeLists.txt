# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(TEST_FILES

	${CMAKE_CURRENT_SOURCE_DIR}/test_sur_les_set.cpp
)

ADD_EXECUTABLE(test_sur_les_set ${TEST_FILES})
TARGET_LINK_LIBRARIES(test_sur_les_set ${PROJECT_NAME}::sequentialcore)
