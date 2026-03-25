#pragma once
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/outer_loop/IMasterUpdate.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBiLevel.h"
#include "common_mpi.h"

namespace Outerloop
{

class CriterionCouldNotBeSatisfied: public LogUtils::XpansionError<std::runtime_error>
{
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopBenders: public OuterLoop
{
public:
    explicit OuterLoopBenders(
      const std::vector<Benders::Criterion::CriterionSingleInputData>& outer_loop_data,
      std::shared_ptr<IMasterUpdate> master_updater,
      pBendersBase benders,
      mpi::communicator& world);

    void Run() override;

    void OuterLoopCheckFeasibility() override;
    void OuterLoopBilevelChecks() override;
    void RunAttachedAlgo() override;
    void PrintLog() override;
    void init_data() override;
    bool isExceptionRaised() override;
    bool UpdateMaster() override;
    ~OuterLoopBenders() override = default;

    // Public getter methods for OuterLoopBiLevel bounds (used by tests and internally)
    double OuterLoopLambdaMin() const;
    double OuterLoopLambdaMax() const;

private:
    std::shared_ptr<IMasterUpdate> master_updater_;
    pBendersBase benders_;
    OuterLoopBendersAdapter adapter_;
    BendersLoggerBase loggers_;
    mpi::communicator& world_;
    bool is_bilevel_check_all_ = false;
    void InitExternalValues(bool is_bilevel_check_all, double lambda);
    OuterLoopBiLevel outer_loop_biLevel_;
};
} // namespace Outerloop
