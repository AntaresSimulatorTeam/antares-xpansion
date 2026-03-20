#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include <antares-xpansion/benders/benders_core/SimulationOptions.h>
#include <variant>

#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

class BendersApp
{
    boost::mpi::communicator* pworld_ = nullptr;
    SOLVER solver_ = SOLVER::BENDERS;
    SimulationOptions options_;
    BendersLoggerBase benders_loggers_;
    std::variant<Benders::Criterion::CriterionInputData,
                 Benders::Criterion::OuterLoopCriterionInputData>
      criterion_input_holder_;
    std::shared_ptr<BendersBase> benders_ = nullptr;
    Logger logger_ = nullptr;
    std::shared_ptr<Output::OutputWriter> writer_ = nullptr;
    std::shared_ptr<MathLoggerDriver> math_log_driver_;
    BENDERSMETHOD method_ = BENDERSMETHOD::BENDERS;
    std::string context_ = bendersmethod_to_string(BENDERSMETHOD::BENDERS);
    std::string positive_unsupplied_file_;
    static constexpr const char* const LOLD_FILE = "LOLD.txt";

    [[nodiscard]] int RunExternalLoop();
    [[nodiscard]] int RunBenders();
    void InitializeBendersEnvironment(bool outer_loop);
    void SetupMathLogger(bool benders_log_console) const;
    void StartMessage();
    void EndMessage(const double execution_time);
    void AddCriterionOutputs();
    bool isCriterionListEmpty() const;
    void SetupLoggerAndOutputWriter(const BendersBaseOptions& benders_options);

public:
    explicit BendersApp(const std::filesystem::path& options_file,
                        boost::mpi::communicator& world,
                        const SOLVER& solver);
    int Run();
    std::filesystem::path LogReportsName() const;
};
#endif // ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
