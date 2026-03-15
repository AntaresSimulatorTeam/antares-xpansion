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
    Benders::Criterion::CriterionInputData criterion_data;
    auto areas = ReadAreaFile();

    for (const auto& area: areas)
    {
        criterion_data.AddSingleData(Benders::Criterion::CriterionSingleInputData(area, "", 0.0));
    }

    return criterion_data;
}

std::set<std::string> BendersFactory::ReadAreaFile()
{
    std::filesystem::path area_file = std::filesystem::path(options_.OUTPUTROOT)
                                      / options_.AREA_FILE;
    auto area_data = AreaParser::ReadAreaFile(area_file);

    if (!area_data.error_message.empty())
    {
        throw std::runtime_error("Error reading area file: " + area_data.error_message);
    }

    return std::set<std::string>(area_data.areas.begin(), area_data.areas.end());
}
