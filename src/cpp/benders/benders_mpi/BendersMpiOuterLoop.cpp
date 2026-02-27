#include "antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h"

#include <utility>

namespace Outerloop
{

BendersMpiOuterLoop::BendersMpiOuterLoop(const BendersBaseOptions& options,
                                         Logger logger,
                                         std::shared_ptr<Output::OutputWriter> writer,
                                         std::shared_ptr<mpi::communicator> world,
                                         std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
    BendersMpi(options, std::move(logger), std::move(writer), std::move(world), std::move(mathLoggerDriver))
{
}

void BendersMpiOuterLoop::launch()
{
    ++_data.criteria_current_iteration_data.benders_num_run;
    BendersMpi::launch();
}
} // namespace Outerloop


