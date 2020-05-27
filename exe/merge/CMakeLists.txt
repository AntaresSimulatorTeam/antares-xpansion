# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)


# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

SET(MERGE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/merge_mps.cpp
)

ADD_EXECUTABLE(merge_mps ${MERGE_FILES})
TARGET_LINK_LIBRARIES(merge_mps 
	PUBLIC
        ${PROJECT_NAME}::sequentialcore
		${XPRESS_LIBRARY}
)
