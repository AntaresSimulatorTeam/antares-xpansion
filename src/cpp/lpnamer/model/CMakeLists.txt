# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (lp_namer_model STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/Candidate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Candidate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ActiveLinks.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ActiveLinks.h
	${CMAKE_CURRENT_SOURCE_DIR}/LinkProfile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LinkProfile.h
		${CMAKE_CURRENT_SOURCE_DIR}/Problem.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Problem.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemNameParser.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemNameParser.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapReader.h
		${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapReader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapProvider.h
		${CMAKE_CURRENT_SOURCE_DIR}/ChronicleMapProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SimulationInputMode.h
)

target_include_directories (lp_namer_model
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (lp_namer_model
	PUBLIC
		antaresXpansion::lp_namer_helper
		antaresXpansion::helpers
		solvers
)

add_library (${PROJECT_NAME}::lp_namer_model ALIAS lp_namer_model)
