#include "OuterLoop.h"
namespace Outerloop {

void OuterLoop::Run() {
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
  PrintLog();
}

OuterLoop::OuterLoop(CriterionComputation &criterion_computation)
    : criterion_computation_(criterion_computation),
      outer_loop_biLevel_(criterion_computation.getOuterLoopInputData()) {}

}  // namespace Outerloop