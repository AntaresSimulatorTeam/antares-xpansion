# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (lp_namer_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/IntercoDataMps.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdater.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LinkdataRecord.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CandidatesInitializer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common_lpnamer.h
	${CMAKE_CURRENT_SOURCE_DIR}/INIReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/IntercoDataMps.h
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.h
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.h
)

target_include_directories (lp_namer_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
)

target_link_libraries (lp_namer_core
	PUBLIC
		helpers
)

add_library (${PROJECT_NAME}::lp_namer_core ALIAS lp_namer_core)
