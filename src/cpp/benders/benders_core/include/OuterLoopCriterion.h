#pragma once
#include <memory>

#include "BendersBase.h"
#include "CutsManagement.h"
class IOuterLoopCriterion {
 public:
  virtual bool Check() = 0;
};

class OuterloopCriterionLOL : public IOuterLoopCriterion {
 public:
  explicit OuterloopCriterionLOL(double threshold, double epsilon){};
  bool Check() override { return true; };
};
