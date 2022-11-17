# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Sequential Benders Exe
# ---------------------------------------------------------------------------

add_executable(lp_namer
		${CMAKE_CURRENT_SOURCE_DIR}/LpsFromAntares.h
		${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
		XpansionProblemsFromAntaresProvider.cpp
		XpansionProblemsFromAntaresProvider.h
		AntaresProblemToXpansionProblemTranslator.cpp
		AntaresProblemToXpansionProblemTranslator.h
		IXpansionProblemsProvider.h ZipProblemsProviderAdapter.cpp ZipProblemsProviderAdapter.h)

find_package(Boost REQUIRED COMPONENTS serialization)
target_link_libraries(lp_namer
		PRIVATE
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		Boost::boost Boost::program_options
		Boost::serialization)

install(TARGETS lp_namer DESTINATION bin)
