#pragma once
#include "MasterUpdate.h"

namespace Outerloop {
class IOuterLoop {
 public:
  virtual void Run() = 0;
  virtual void OuterLoopCheckFeasibility() = 0;
  virtual void OuterLoopBilevelChecks() = 0;

};

}  // namespace Outerloop