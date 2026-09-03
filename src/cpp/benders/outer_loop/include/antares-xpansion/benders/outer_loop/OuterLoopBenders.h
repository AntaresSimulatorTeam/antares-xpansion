#pragma once
#include <memory>
#include <vector>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"
#include "antares-xpansion/benders/outer_loop/IMasterUpdate.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBiLevel.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"

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
      std::shared_ptr<ICommunicationStrategy> communication_strategy);

    void Run() override;

    void OuterLoopCheckFeasibility() override;
    void OuterLoopBilevelChecks() override;
    void RunAttachedAlgo() override;
    void PrintLog() override;
    void init_data() override;
    bool isExceptionRaised() override;
    double OuterLoopLambdaMin() const;
    double OuterLoopLambdaMax() const;
    bool UpdateMaster() override;
    ~OuterLoopBenders() override = default;

private:
    std::shared_ptr<IMasterUpdate> master_updater_;
    std::shared_ptr<OuterLoopBendersAdapter> adapter_;
    std::shared_ptr<ICommunicationStrategy> communication_strategy_;
    BendersLoggerBase loggers_;
    bool is_bilevel_check_all_ = false;
    void InitExternalValues(bool is_bilevel_check_all, double lambda);
    OuterLoopBiLevel outer_loop_biLevel_;
};
} // namespace Outerloop
