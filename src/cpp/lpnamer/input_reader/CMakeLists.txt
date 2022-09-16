# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (lp_namer_input_reader STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/INIReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/CandidatesINIReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CandidatesINIReader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LinkProfileReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/LinkProfileReader.h
	${CMAKE_CURRENT_SOURCE_DIR}/VariableFileReader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/VariableFileReader.h
        IntercoFileReader.cpp IntercoFileReader.h)

target_include_directories (lp_namer_input_reader
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (lp_namer_input_reader
	PUBLIC
	    lp_namer_model
		helpers
		solvers
)

add_library (${PROJECT_NAME}::lp_namer_input_reader ALIAS lp_namer_input_reader)
