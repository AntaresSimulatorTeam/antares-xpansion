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
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersMathLogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersProblemFromFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersStructsDatas.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationPrinter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LastIterationWriter.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/SolverRowExtractor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SkeletonSolverLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/skeleton_coefficient_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SkeletonCoefficientSet.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SkeletonConstraintSetLoader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SubproblemBasisCache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SubproblemWorkerFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SubproblemConstraintsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersBase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersMathLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersProblemFromFile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersMethod.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/BendersStructsDatas.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CriterionComputation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CriterionInputDataReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/CustomVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/ExecutionPolicy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/IBendersPlugin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/IBendersProblemProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationPrinter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/LastIterationWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SimulationOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SolverIO.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/StartUp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SubproblemWorker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/VariablesGroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/Worker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/skeleton_coefficient_reader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SkeletonCoefficientSet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SkeletonConstraintSetLoader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SubproblemBasisCache.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/WorkerMaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SolverRowExtractor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SkeletonSolverLoader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SubproblemConstraintsManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/ICommunicationStrategy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_core/SequentialCommunicationStrategy.h
)

add_library(antaresXpansion::benders_core ALIAS benders_core)

target_include_directories(benders_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

find_package(fmt REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
target_link_libraries(benders_core
        PUBLIC
        antaresXpansion::helpers
        antaresXpansion::output_core
        antaresXpansion::solvers
        ${JSONCPP_LIB}
        antaresXpansion::core
        fmt::fmt
        yaml-cpp::yaml-cpp
)

target_link_libraries(benders_core PRIVATE TBB::tbb)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)