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

}  // namespace Outerloop