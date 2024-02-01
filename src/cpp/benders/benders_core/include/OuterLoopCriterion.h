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
  explicit OuterloopCriterionLOL(const BendersCuts& bendersCuts,
                                 double threshold, double epsilon);
  bool Check() override { return true; };

 private:
  const BendersCuts& benders_cuts_;
  double threshold_ = 1e6;
  double epsilon_ = 1e-4;
};
