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

class IMasterUpdate {
 public:
  virtual void Update() = 0;
};

class OuterLoop {
 public:
  explicit OuterLoop(std::shared_ptr<IOuterLoopCriterion> criterion,
                     std::shared_ptr<IMasterUpdate> master_updater,
                     pBendersBase benders);
  void Run();

 private:
  std::shared_ptr<IOuterLoopCriterion> criterion_;
  std::shared_ptr<IMasterUpdate> master_updater_;
  pBendersBase benders_;
};