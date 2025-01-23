#include <utility>

#include "antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h"


namespace Outerloop {

BendersMpiOuterLoop::BendersMpiOuterLoop(
    const BendersBaseOptions& options, Logger logger, std::shared_ptr<Output::OutputWriter> writer,
    mpi::environment& env, mpi::communicator& world,
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver)
    : BendersMpi(options, std::move(logger), std::move(writer), env, world, std::move(mathLoggerDriver)) {}


void BendersMpiOuterLoop::launch() {
  ++_data.criteria_current_iteration_data.benders_num_run;
  BendersMpi::launch();
}
}  // namespace Outerloop