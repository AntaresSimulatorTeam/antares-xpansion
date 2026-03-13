#pragma once
#include "BendersMPI.h"
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

namespace Outerloop
{

/*!
 * \class BendersMpiOuterLoop
 * \brief Outer loop variant of Benders
 * \deprecated Use BendersCore with OuterLoopStrategy instead - will be removed in future version
 */
class [[deprecated("Use BendersCore with OuterLoopStrategy instead")]] BendersMpiOuterLoop
    : public BendersMpi
{
public:
    ~BendersMpiOuterLoop() override = default;
    BendersMpiOuterLoop(const BendersBaseOptions& options,
                        Logger logger,
                        std::shared_ptr<Output::OutputWriter> writer,
                        mpi::communicator& world,
                        std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    void launch() override;
};

} // namespace Outerloop
