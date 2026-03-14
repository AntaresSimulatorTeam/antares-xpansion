# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package(TBB REQUIRED CONFIG)

add_library(benders_core)
target_sources(benders_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersCore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersMathLogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersProblemFromFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersStructsDatas.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CutsManagement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationPrinter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MasterUpdateBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SimulationOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SolverIO.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/StartUp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SubproblemWorker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Worker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WorkerMaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CriterionComputation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CriterionInputDataReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/VariablesGroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CouplingMapGenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/MPISubproblemSolver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/SequentialSubproblemSolver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/BatchSubproblemSolver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/SingleLoopStrategy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/OuterLoopStrategy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/StandardBatchStrategy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersBase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersCore.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersMathLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersProblemFromFile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersMethod.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersStructsDatas.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CriterionComputation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CriterionInputDataReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CustomVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CutsManagement.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/IBendersProblemProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationPrinter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/MasterUpdate.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SimulationOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SolverIO.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/StartUp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SubproblemWorker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/VariablesGroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/Worker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/WorkerMaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/ISubproblemSolver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/ILoopStrategy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/MPISubproblemSolver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/SequentialSubproblemSolver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/BatchSubproblemSolver.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/SingleLoopStrategy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/OuterLoopStrategy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/IBatchStrategy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/strategies/include/antares-xpansion/benders/benders_core/strategies/StandardBatchStrategy.h
)

add_library(antaresXpansion::benders_core ALIAS benders_core)

target_include_directories(benders_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/strategies/include>
)

find_package(fmt REQUIRED)
target_link_libraries(benders_core
        PUBLIC
        antaresXpansion::helpers
        antaresXpansion::output_core
        antaresXpansion::solvers
        ${JSONCPP_LIB}
        antaresXpansion::outer_loop_lib
        antaresXpansion::core
        antaresXpansion::plugins
        fmt::fmt
)

target_link_libraries(benders_core PRIVATE TBB::tbb)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)