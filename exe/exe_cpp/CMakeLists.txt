# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)


# ---------------------------------------------------------------------------
# Sequential Benders Exe
# ---------------------------------------------------------------------------
SET(EXE_CPP_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

ADD_EXECUTABLE(benderssequential ${EXE_CPP_FILES})
TARGET_LINK_LIBRARIES(benderssequential ${PROJECT_NAME}::sequentialcore ${XPRESS_LIBRARY})
