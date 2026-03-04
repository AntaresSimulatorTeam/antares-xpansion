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
        ${CMAKE_CURRENT_SOURCE_DIR}/ConfigurationManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGeneration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationForWaterValueCalculation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ConfigurationManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ProblemGeneration.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/ProblemGenerationOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Version.h
)

target_link_libraries(problem_generation_main
        PRIVATE
        ${PROJECT_NAME}::lp_namer_model
        ${PROJECT_NAME}::lp_namer_problem_modifier
        Boost::boost Boost::program_options
        antaresXpansion::helpers
        antaresXpansion::xpansion_interfaces
        Antares::single_problem_api
        Antares::solver_api
        Antares::file-tree-study-loader
        PUBLIC
        ${PROJECT_NAME}::lp_namer_helper
        ${PROJECT_NAME}::lp_namer_input_reader
        ${PROJECT_NAME}::grid_evaluator_lib
        Antares::antares-solver-simulation
        ${PROJECT_NAME}::core
        ${PROJECT_NAME}::bellman_values_lib
)

target_include_directories(problem_generation_main
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
)


add_library(${PROJECT_NAME}::problem_generation_main ALIAS problem_generation_main)

#Multiple problem generation lib
add_library(multiple_problem_generation_lib)
target_sources(multiple_problem_generation_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/main/MultipleProblemGeneration.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MultipleProblemGeneration.cpp
)

target_link_libraries(multiple_problem_generation_lib PUBLIC
        ${PROJECT_NAME}::problem_generation_main
        ${PROJECT_NAME}::node_lp_data_location
)

target_include_directories(multiple_problem_generation_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::multiple_problem_generation_lib ALIAS multiple_problem_generation_lib)


install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
