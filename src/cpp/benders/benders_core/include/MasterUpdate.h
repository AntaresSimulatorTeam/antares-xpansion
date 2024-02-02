#pragma once
#include "OuterLoopCriterion.h"

class IMasterUpdate {
 public:
  virtual void Update(const CRITERION& criterion, pBendersBase benders) = 0;
  virtual void AddConstraints() = 0;
  virtual void AddCutsInMaster() = 0;
};

class MasterUpdateBase : public IMasterUpdate {
 public:
  explicit MasterUpdateBase(double lambda, double lambda_min, double lambda_max,
                            double tau);
  void Update(const CRITERION& criterion, pBendersBase benders) override;
  void AddConstraints() override;
  void AddCutsInMaster() override;

 private:
  double lambda_ = 0;
  double lambda_min_ = 0;
  double lambda_max_ = 1;
  double tau_ = 0.5;
};
