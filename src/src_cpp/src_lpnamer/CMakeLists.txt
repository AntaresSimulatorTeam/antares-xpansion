# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (lpnamer STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/IntercoDataMps.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common_lpnamer.h
	${CMAKE_CURRENT_SOURCE_DIR}/INIReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/IntercoDataMps.h
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.h
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.h
)

target_include_directories (lpnamer
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
)

target_link_libraries (lpnamer
	PUBLIC
		helpers
)

add_library (${PROJECT_NAME}::lpnamer ALIAS lpnamer)
