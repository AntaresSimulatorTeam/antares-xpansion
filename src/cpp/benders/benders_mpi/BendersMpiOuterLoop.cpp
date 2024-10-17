#include "antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h"

#include "antares-xpansion/benders/benders_core/CustomVector.h"

namespace Outerloop {

BendersMpiOuterLoop::BendersMpiOuterLoop(
    const BendersBaseOptions& options, Logger logger, Writer writer,
    mpi::environment& env, mpi::communicator& world,
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver)
    : BendersMpi(options, logger, writer, env, world, mathLoggerDriver) {}


void BendersMpiOuterLoop::launch() {
  ++_data.outer_loop_current_iteration_data.benders_num_run;
  BendersMpi::launch();
}
}  // namespace Outerloop