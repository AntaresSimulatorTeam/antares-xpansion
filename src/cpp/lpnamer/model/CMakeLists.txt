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
        Problem.cpp Problem.h)

target_include_directories (lp_namer_model
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (lp_namer_model
	PUBLIC
	    lp_namer_helper
		helpers
		solvers
)

add_library (${PROJECT_NAME}::lp_namer_model ALIAS lp_namer_model)
