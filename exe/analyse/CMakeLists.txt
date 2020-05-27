# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# ---------------------------------------------------------------------------
# Analyse MPS Exe
# ---------------------------------------------------------------------------

SET(ANALYSE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/analyse_mps.cpp
)

ADD_EXECUTABLE(analyse_mps ${ANALYSE_FILES})
TARGET_LINK_LIBRARIES(analyse_mps ${PROJECT_NAME}::sequentialcore ${XPRESS_LIBRARY})