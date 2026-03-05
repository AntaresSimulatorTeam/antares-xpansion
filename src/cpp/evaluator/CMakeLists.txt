# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(evaluator_lib STATIC)

target_sources(evaluator_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Evaluator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GridEvaluator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GridCollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirManagement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/evaluator/Evaluator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/evaluator/GridEvaluator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/evaluator/GridCollection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/evaluator/ReservoirManagement.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/evaluator/Interpolator.h
)

target_link_libraries(evaluator_lib
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::output_core
        Antares::solver_api
        antaresXpansion::lp_namer_model
)

target_include_directories(evaluator_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/cpp/xpansion_interfaces/include>
)

add_library(${PROJECT_NAME}::evaluator_lib ALIAS evaluator_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
