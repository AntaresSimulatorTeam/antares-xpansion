#pragma once
#include <antares-xpansion/benders/benders_core/BendersMethod.h>
#include <antares-xpansion/benders/benders_core/CriterionInputDataReader.h>
#include <antares-xpansion/benders/benders_core/common.h>
#include <memory>
#include <variant>
#include <optional>

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
class BendersBase;
class SimulationOptions;
class BendersBaseOptions;

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

    BendersFactory(const SimulationOptions& options,
                   std::shared_ptr<ILogger> logger,
                   std::shared_ptr<Output::OutputWriter> writer,
                   std::shared_ptr<MathLoggerDriver> math_log_driver_,
                   int rank,
                   boost::mpi::environment* env,
                   boost::mpi::communicator* world,
                   BendersLoggerBase& benders_loggers);
    auto PrepareForExecution(bool outer_loop) -> std::optional<BendersEnvironment>;

private:
    auto ConfigureBenders(const BendersBaseOptions& benders_options,
                          const CouplingMap& coupling_map) -> BendersEnvironment;
    [[nodiscard]] std::variant<Benders::Criterion::CriterionInputData,
                               Benders::Criterion::OuterLoopCriterionInputData>
    ProcessCriterionInput();
    Benders::Criterion::CriterionInputData BuildPatternsUsingAreaFile();
    std::set<std::string> ReadAreaFile();
    void ConfigureSolverLog(BendersBase* benders);

    const SimulationOptions& options_;
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<Output::OutputWriter> writer_;
    std::shared_ptr<MathLoggerDriver> math_log_driver_;
    boost::mpi::environment* env_ = nullptr;
    boost::mpi::communicator* world_ = nullptr;
    int rank = 0;
    BENDERSMETHOD method_;
    BendersLoggerBase& benders_loggers_;
    std::string context_ = bendersmethod_to_string(BENDERSMETHOD::BENDERS);
    static constexpr const char* const LOLD_FILE = "LOLD.txt";
};
