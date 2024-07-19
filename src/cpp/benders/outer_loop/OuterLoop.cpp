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

OuterLoop::OuterLoop(const OuterLoopInputData& outer_loop_input_data)
    : criterion_computation_(outer_loop_input_data),
      outer_loop_biLevel_(outer_loop_input_data) {}

}  // namespace Outerloop