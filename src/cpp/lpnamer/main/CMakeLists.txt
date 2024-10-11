# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation  runner lib
# ---------------------------------------------------------------------------

find_package(Antares REQUIRED)
add_library(problem_generation_main)
target_sources(problem_generation_main PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGeneration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ProblemGeneration.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ProblemGenerationOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Version.h
)

target_link_libraries(problem_generation_main
        PRIVATE
        ${PROJECT_NAME}::lp_namer_helper
        ${PROJECT_NAME}::lp_namer_model
        ${PROJECT_NAME}::lp_namer_input_reader
        ${PROJECT_NAME}::lp_namer_problem_modifier
        Boost::boost Boost::program_options
        antaresXpansion::helpers
        Antares::solver_api
        PUBLIC
        Antares::antares-solver-simulation
)

get_target_property(helpers_include helpers INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(problem_generation_main
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
        ${helpers_include}
)


add_library(${PROJECT_NAME}::problem_generation_main ALIAS problem_generation_main)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)