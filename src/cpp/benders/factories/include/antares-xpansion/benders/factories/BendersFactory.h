#pragma once
#include <antares-xpansion/benders/benders_core/BendersBase.h>
#include <antares-xpansion/benders/benders_core/BendersCore.h>
#include <antares-xpansion/benders/benders_core/BendersMethod.h>
#include <antares-xpansion/benders/benders_core/CriterionInputDataReader.h>
#include <antares-xpansion/benders/benders_core/common.h>
#include <antares-xpansion/benders/factories/BendersPluginFactory.h>
#include <antares-xpansion/benders/plugins/BendersPlugin.h>
#include <memory>
#include <optional>
#include <variant>

struct BendersLoggerBase;
class MathLoggerDriver;

namespace boost::mpi
{
class communicator;
class environment;
} // namespace boost::mpi

namespace Output
{
class OutputWriter;
}
class ILogger;
class SimulationOptions;
struct BendersBaseOptions;

class BendersFactory
{
public:
    struct BendersEnvironment
    {
        std::unique_ptr<BendersBase> benders{nullptr};
        std::variant<Benders::Criterion::CriterionInputData,
                     Benders::Criterion::OuterLoopCriterionInputData>
          criterion_input_data;
        BENDERSMETHOD method{BENDERSMETHOD::BENDERS};
    };

    struct Dependencies
    {
        std::shared_ptr<ILogger> logger;
        std::shared_ptr<Output::OutputWriter> writer;
        std::shared_ptr<MathLoggerDriver> math_log_driver;
        BendersLoggerBase& benders_loggers;
    };

    BendersFactory(const SimulationOptions& options,
                   boost::mpi::communicator* world,
                   Dependencies dependencies);
    auto PrepareForExecution(bool outer_loop) -> std::optional<BendersEnvironment>;

private:
    auto ConfigureBenders(const BendersBaseOptions& benders_options,
                          const CouplingMap& coupling_map) -> BendersEnvironment;
    [[nodiscard]] std::variant<Benders::Criterion::CriterionInputData,
                               Benders::Criterion::OuterLoopCriterionInputData>
    ProcessCriterionInput(bool is_batch, bool is_outer_loop);
    Benders::Criterion::CriterionInputData BuildPatternsUsingAreaFile();
    std::set<std::string> ReadAreaFile();
    void ConfigureSolverLog(BendersBase* benders);

    const SimulationOptions& options_;
    Dependencies dependencies_;
    boost::mpi::communicator* world_ = nullptr;
    std::shared_ptr<BendersPluginFactory> benders_plugin_factory_;
    int rank = 0;
    BENDERSMETHOD method_;
    std::string context_;
    static constexpr const char* const LOLD_FILE = "LOLD.txt";
};
