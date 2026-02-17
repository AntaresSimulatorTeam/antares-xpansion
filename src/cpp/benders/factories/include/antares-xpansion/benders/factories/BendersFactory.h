#pragma once
#include <antares-xpansion/benders/benders_core/BendersBase.h>
#include <antares-xpansion/benders/benders_core/BendersMethod.h>
#include <antares-xpansion/benders/benders_core/CriterionInputDataReader.h>
#include <antares-xpansion/benders/benders_core/common.h>
#include <memory>
#include <optional>
#include <variant>

#ifdef ENABLE_BENDERS_STRATEGY
#include <antares-xpansion/benders/strategy/IBendersCore.h>
#endif

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

#ifdef ENABLE_BENDERS_STRATEGY
    struct StrategyBendersEnvironment
    {
        std::unique_ptr<IBendersCore> benders{nullptr};
        std::variant<Benders::Criterion::CriterionInputData,
                     Benders::Criterion::OuterLoopCriterionInputData>
          criterion_input_data;
        BENDERSMETHOD method{BENDERSMETHOD::BENDERS};
    };
#endif

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

#ifdef ENABLE_BENDERS_STRATEGY
    /**
     * @brief Create Benders implementation using Strategy pattern
     * @param outer_loop Whether to use outer loop optimization
     * @return Environment with strategy-based BendersCore implementation
     * 
     * This method builds a BendersCore by composing ExecutionStrategy,
     * BatchingStrategy, and OuterLoopStrategy based on configuration options.
     */
    auto PrepareForExecutionWithStrategies(bool outer_loop) -> std::optional<StrategyBendersEnvironment>;
#endif

private:
    auto ConfigureBenders(const BendersBaseOptions& benders_options,
                          const CouplingMap& coupling_map) -> BendersEnvironment;
#ifdef ENABLE_BENDERS_STRATEGY
    auto ConfigureBendersWithStrategies(const BendersBaseOptions& benders_options,
                                       const CouplingMap& coupling_map) -> StrategyBendersEnvironment;
#endif
    [[nodiscard]] std::variant<Benders::Criterion::CriterionInputData,
                               Benders::Criterion::OuterLoopCriterionInputData>
    ProcessCriterionInput();
    Benders::Criterion::CriterionInputData BuildPatternsUsingAreaFile();
    std::set<std::string> ReadAreaFile();
    void ConfigureSolverLog(BendersBase* benders);

    const SimulationOptions& options_;
    Dependencies dependencies_;
    boost::mpi::communicator* world_ = nullptr;
    int rank = 0;
    BENDERSMETHOD method_;
    std::string context_ = bendersmethod_to_string(BENDERSMETHOD::BENDERS);
    static constexpr const char* const LOLD_FILE = "LOLD.txt";
};
