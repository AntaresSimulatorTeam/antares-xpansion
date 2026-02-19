#include "antares-xpansion/benders/factories/BendersFactory.h"

#include <antares-xpansion/benders/benders_by_batch/BendersByBatch.h>
#include <antares-xpansion/benders/benders_core/BendersBase.h>
#include <antares-xpansion/benders/benders_core/BendersMethod.h>
#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/benders/benders_core/StartUp.h>
#include <antares-xpansion/benders/benders_core/common.h>
#include <antares-xpansion/benders/benders_mpi/BendersMPI.h>
#include <antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h>
#include <antares-xpansion/benders/benders_sequential/BendersSequential.h>
#include <antares-xpansion/benders/outer_loop/OuterLoopBiLevel.h>
#include <antares-xpansion/benders/strategy/BendersCore.h>
#include <antares-xpansion/benders/strategy/ByBatchStrategy.h>
#include <antares-xpansion/benders/strategy/NoBatchingStrategy.h>
#include <antares-xpansion/benders/strategy/NoOuterLoopStrategy.h>
#include <antares-xpansion/benders/strategy/OuterLoopAdapter.h>
#include <antares-xpansion/benders/strategy/ParallelMpiExecutionStrategy.h>
#include <antares-xpansion/benders/strategy/SequentialExecutionStrategy.h>
#include <antares-xpansion/helpers/AreaParser.h>
#include <variant>

BendersFactory::BendersFactory(const SimulationOptions& options,
                               boost::mpi::communicator* world,
                               Dependencies dependencies):
    options_{options},
    dependencies_{dependencies},
    world_{world},
    rank{world->rank()}
{
}

BENDERSMETHOD DeduceBendersMethod(size_t coupling_map_size, size_t batch_size, bool outer_loop)
{
    if (batch_size == 0 || batch_size == coupling_map_size - 1)
    {
        if (outer_loop)
        {
            return BENDERSMETHOD::BENDERS_OUTERLOOP;
        }
        return BENDERSMETHOD::BENDERS;
    }
    if (outer_loop)
    {
        return BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP;
    }
    return BENDERSMETHOD::BENDERS_BY_BATCH;
}

std::variant<Benders::Criterion::CriterionInputData,
             Benders::Criterion::OuterLoopCriterionInputData>
BendersFactory::ProcessCriterionInput()
{
    const auto fpath = std::filesystem::path(options_.INPUTROOT) / options_.OUTER_LOOP_OPTION_FILE;
    // if adequacy_criterion.yml is provided read it
    if ((method_ == BENDERSMETHOD::BENDERS_OUTERLOOP
         || method_ == BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP)
        && std::filesystem::exists(fpath))
    {
        return Benders::Criterion::CriterionInputFromYaml().Read(fpath);
    }
    // else compute criterion for all areas!
    else
    {
        return BuildPatternsUsingAreaFile();
    }
}

Benders::Criterion::CriterionInputData BendersFactory::BuildPatternsUsingAreaFile()
{
    std::set<std::string> unique_areas = ReadAreaFile();
    Benders::Criterion::CriterionInputData ret;
    ret.SetCriterionCountThreshold(1);

    for (const auto& area: unique_areas)
    {
        Benders::Criterion::CriterionSingleInputData
          singleInputData(Benders::Criterion::PositiveUnsuppliedEnergy, area, 1);
        ret.AddSingleData(singleInputData);
    }

    return ret;
}

std::set<std::string> BendersFactory::ReadAreaFile()
{
    std::set<std::string> unique_areas;
    const auto area_file = std::filesystem::path(options_.INPUTROOT) / options_.AREA_FILE;
    const auto area_file_data = AreaParser::ReadAreaFile(area_file);
    if (const auto& msg = area_file_data.error_message; !msg.empty())
    {
        dependencies_.benders_loggers.display_message(msg, LogUtils::LOGLEVEL::WARNING, context_);
        std::ostringstream ms;
        ms << " Consequently, " << LOLD_FILE
           << " and other criterion based files will not be produced!";

        dependencies_.benders_loggers.display_message(ms.str(),
                                                      LogUtils::LOGLEVEL::WARNING,
                                                      context_);
        return {};
    }
    return {area_file_data.areas.begin(), area_file_data.areas.end()};
}

auto BendersFactory::ConfigureBenders(const BendersBaseOptions& benders_options,
                                      const CouplingMap& coupling_map) -> BendersEnvironment
{
    // Determine which strategies to create based on method
    bool use_batching = (method_ == BENDERSMETHOD::BENDERS_BY_BATCH
                         || method_ == BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP);
    bool use_outer_loop = (method_ == BENDERSMETHOD::BENDERS_OUTERLOOP
                           || method_ == BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP);

    // Build ExecutionStrategy
    // Use Sequential if running with single process (world size == 1), otherwise use MPI
    std::unique_ptr<IExecutionStrategy> execution_strategy;

    if (world_->size() == 1)
    {
        // Sequential execution (single process)
        auto sequential_benders = std::make_unique<BendersSequential>(
          benders_options,
          dependencies_.logger,
          dependencies_.writer,
          dependencies_.math_log_driver);
        execution_strategy = std::make_unique<SequentialExecutionStrategy>(
          std::move(sequential_benders));
    }
    else
    {
        // MPI parallel execution (multiple processes)
        auto mpi_benders = std::make_unique<BendersMpi>(benders_options,
                                                        dependencies_.logger,
                                                        dependencies_.writer,
                                                        *world_,
                                                        dependencies_.math_log_driver);
        execution_strategy = std::make_unique<ParallelMpiExecutionStrategy>(std::move(mpi_benders));
    }

    // Build BatchingStrategy
    std::unique_ptr<IBatchingStrategy> batching_strategy;
    if (use_batching)
    {
        auto batch_benders = std::make_unique<BendersByBatch>(benders_options,
                                                              dependencies_.logger,
                                                              dependencies_.writer,
                                                              *world_,
                                                              dependencies_.math_log_driver);
        batching_strategy = std::make_unique<ByBatchStrategy>(std::move(batch_benders));
    }
    else
    {
        batching_strategy = std::make_unique<NoBatchingStrategy>();
    }

    // Build OuterLoopStrategy
    std::unique_ptr<IOuterLoopStrategy> outer_loop_strategy;
    // For now avoid constructing complex outer-loop objects here (requires
    // BendersBase/pBendersBase wiring). Use NoOuterLoopStrategy to keep build
    // stable; actual outer-loop wiring happens in RunExternalLoop paths.
    outer_loop_strategy = std::make_unique<NoOuterLoopStrategy>();

    // Compose strategies into BendersCore
    auto benders_core = std::make_unique<BendersCore>(std::move(execution_strategy),
                                                      std::move(batching_strategy),
                                                      std::move(outer_loop_strategy));

    // Set input map via BendersCore interface (now properly delegated)
    benders_core->set_input_map(coupling_map);

    // NOTE: do not call benders_core->set_solver_log_file here - BendersCore
    // doesn't expose that API. Solver log configuration is handled by
    // ConfigureSolverLog which dynamic_casts to BendersBase when available.

    auto criterion_input_holder = ProcessCriterionInput();
    // setCriterionComputationInputs was removed; criterion data is stored in environment
    return BendersEnvironment{std::move(benders_core), criterion_input_holder, method_};
}

void BendersFactory::ConfigureSolverLog(IBendersCore* benders)
{
    if (options_.LOG_LEVEL > 1 && benders)
    {
        auto solver_log = std::filesystem::path(options_.OUTPUTROOT)
                          / (std::string("solver_log_proc_") + std::to_string(world_->rank())
                             + ".txt");
        if (auto base = dynamic_cast<BendersBase*>(benders))
        {
            base->set_solver_log_file(solver_log);
        }
    }
}

auto BendersFactory::PrepareForExecution(bool outer_loop) -> std::optional<BendersEnvironment>
{
    BendersBaseOptions benders_options(options_.get_benders_options());
    benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = outer_loop;

    const auto coupling_map = CouplingMapGenerator::BuildInput(benders_options.STRUCTURE_FILE,
                                                               dependencies_.logger.get(),
                                                               "Benders");

    method_ = DeduceBendersMethod(coupling_map.size(), options_.BATCH_SIZE, outer_loop);
    context_ = bendersmethod_to_string(method_);

    if (rank == 0)
    {
        if (Benders::StartUp startup;
            startup.StudyAlreadyAchievedCriterion(options_,
                                                  dependencies_.writer.get(),
                                                  dependencies_.logger.get()))
        {
            return {};
        }
    }

    auto environment = ConfigureBenders(benders_options, coupling_map);
    ConfigureSolverLog(environment.benders.get());
    return std::optional<BendersEnvironment>(std::move(environment));
}
