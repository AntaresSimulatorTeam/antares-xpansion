#pragma once
#include "BendersMPI.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

namespace Outerloop
{

class BendersMpiOuterLoop: public BendersMpi
{
public:
    ~BendersMpiOuterLoop() override = default;
    BendersMpiOuterLoop(const BendersBaseOptions& options,
                        Logger logger,
                        std::shared_ptr<Output::OutputWriter> writer,
                        mpi::environment& env,
                        mpi::communicator& world,
                        std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    void launch() override;
};

} // namespace Outerloop
