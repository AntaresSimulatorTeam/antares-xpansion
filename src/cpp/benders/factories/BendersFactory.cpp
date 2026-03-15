#include "antares-xpansion/benders/factories/BendersFactory.h"

#include <antares-xpansion/benders/benders_by_batch/BendersByBatch.h>
#include <antares-xpansion/benders/benders_core/BendersCore.h>
#include <antares-xpansion/benders/benders_core/BendersMethod.h>
#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/benders/benders_core/StartUp.h>
#include <antares-xpansion/benders/benders_core/common.h>
#include <antares-xpansion/benders/benders_core/strategies/BatchSubproblemSolver.h>
#include <antares-xpansion/benders/benders_core/strategies/MPISubproblemSolver.h>
#include <antares-xpansion/benders/benders_core/strategies/OuterLoopStrategy.h>
#include <antares-xpansion/benders/benders_core/strategies/SequentialSubproblemSolver.h>
#include <antares-xpansion/benders/benders_core/strategies/SingleLoopStrategy.h>
#include <antares-xpansion/benders/benders_core/strategies/StandardBatchStrategy.h>
#include <antares-xpansion/helpers/AreaParser.h>
#include <variant>

BendersFactory::BendersFactory(const SimulationOptions& options,
                               boost::mpi::communicator* world,
                               Dependencies dependencies):
    options_{options},
    world_{world},
    rank{world->rank()},
    dependencies_{dependencies}
{
    benders_plugin_factory_ = std::make_shared<BendersPluginFactory>(options);
}

std::string GetMethodName(bool is_batch, bool is_outer_loop)
{
    if (is_batch)
    {
        return is_outer_loop ? "Benders by batch outerloop" : "Benders by batch";
    }
    return is_outer_loop ? "Outerloop around Benders" : "Benders";
}

BENDERSMETHOD GetBendersMethod(bool is_batch, bool is_outer_loop)
{
    if (is_batch)
    {
        return is_outer_loop ? BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP
                             : BENDERSMETHOD::BENDERS_BY_BATCH;
    }
    return is_outer_loop ? BENDERSMETHOD::BENDERS_OUTERLOOP : BENDERSMETHOD::BENDERS;
}

auto BendersFactory::ConfigureBenders(const BendersBaseOptions& benders_options,
                                      const CouplingMap& coupling_map) -> BendersEnvironment
{
    const bool is_batch = (benders_options.BATCH_SIZE > 0
                           && benders_options.BATCH_SIZE != coupling_map.size() - 1);
    const bool is_outer_loop = benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP;

    std::unique_ptr<BendersBase> benders;

    if (is_batch)
    {
        benders = std::make_unique<BendersByBatch>(benders_options,
                                                   dependencies_.logger,
                                                   dependencies_.writer,
                                                   *world_,
                                                   dependencies_.math_log_driver);
    }
    else
    {
        SubproblemSolverPtr solver_strategy = std::make_unique<MPISubproblemSolver>();
        LoopStrategyPtr loop_strategy;
        if (is_outer_loop)
        {
            loop_strategy = std::make_unique<OuterLoopStrategy>();
        }
        else
        {
            loop_strategy = std::make_unique<SingleLoopStrategy>();
        }
        BatchStrategyPtr batch_strategy = std::make_unique<StandardBatchStrategy>();

        benders = std::make_unique<BendersCore>(benders_options,
                                                dependencies_.logger,
                                                dependencies_.writer,
                                                dependencies_.math_log_driver,
                                                world_,
                                                std::move(solver_strategy),
                                                std::move(loop_strategy),
                                                std::move(batch_strategy));
    }

    std::shared_ptr<BendersPlugin> benders_plugin(
      benders_plugin_factory_->CreatePlugin(coupling_map, false, world_));
    benders->SetPlugin(benders_plugin);

    benders->set_input_map(coupling_map);
    auto criterion_input_holder = ProcessCriterionInput(is_batch, is_outer_loop);
    benders->setCriterionComputationInputs(
      std::visit([](auto&& the_variant)
                 { return static_cast<Benders::Criterion::CriterionInputData>(the_variant); },
                 criterion_input_holder));
    method_ = GetBendersMethod(is_batch, is_outer_loop);
    return BendersEnvironment{std::move(benders), criterion_input_holder, method_};
}

void BendersFactory::ConfigureSolverLog(BendersBase* benders)
{
    if (options_.LOG_LEVEL > 1)
    {
        auto solver_log = std::filesystem::path(options_.OUTPUTROOT)
                          / (std::string("solver_log_proc_") + std::to_string(world_->rank())
                             + ".txt");

        benders->set_solver_log_file(solver_log);
    }
}

auto BendersFactory::PrepareForExecution(bool outer_loop) -> std::optional<BendersEnvironment>
{
    BendersBaseOptions benders_options(options_.get_benders_options());
    benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = outer_loop;

    const auto coupling_map = CouplingMapGenerator::BuildInput(benders_options.STRUCTURE_FILE,
                                                               dependencies_.logger.get(),
                                                               "Benders");

    bool is_batch = (options_.BATCH_SIZE > 0 && options_.BATCH_SIZE != coupling_map.size() - 1);
    context_ = GetMethodName(is_batch, outer_loop);

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

auto BendersFactory::ProcessCriterionInput(bool is_batch, bool is_outer_loop)
  -> std::variant<Benders::Criterion::CriterionInputData,
                  Benders::Criterion::OuterLoopCriterionInputData>
{
    if (is_outer_loop)
    {
        std::filesystem::path criterion_file = std::filesystem::path(options_.OUTPUTROOT)
                                               / "criterionOptimisation.yaml";
        Benders::Criterion::CriterionInputFromYaml reader;
        return reader.Read(criterion_file);
    }
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
