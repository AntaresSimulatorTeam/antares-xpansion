#include "antares-xpansion/benders/outer_loop/OuterLoop.h"

#include "antares-xpansion/helpers/Timer.h"

namespace Outerloop {

void OuterLoop::Run() {
  Timer time_counter;
  OuterLoopCheckFeasibility();

  bool stop_update_master = false;
  while (!stop_update_master) {
    PrintLog();
    init_data();
    RunAttachedAlgo();
    if (!isExceptionRaised()) {
      OuterLoopBilevelChecks();
      stop_update_master = UpdateMaster();
    } else {
      stop_update_master = true;
    }
  }
  runtime_ = time_counter.elapsed();
  PrintLog();
}

OuterLoop::OuterLoop(CriterionComputation &criterion_computation)
    : criterion_computation_(criterion_computation),
      outer_loop_biLevel_(
          criterion_computation.getOuterLoopInputData().OuterLoopData()) {}

double OuterLoop::Runtime() const { return runtime_; }

}  // namespace Outerloop