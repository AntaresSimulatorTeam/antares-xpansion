#pragma once
#include "OuterLoopCriterion.h"

class IMasterUpdate {
 public:
  virtual void Update(const CRITERION &criterion) = 0;

  // // à generaliser
  // virtual void AddConstraints() = 0;
  // à deplacer
  // virtual void AddCutsInMaster() = 0;
};

class MasterUpdateBase : public IMasterUpdate {
 public:
  explicit MasterUpdateBase(pBendersBase benders, double lambda,
                            double lambda_min, double lambda_max, double tau);
  void Update(const CRITERION &criterion) override;

 private:
  void UpdateConstraints(pBendersBase benders);
  // void AddCutsInMaster() override;

  pBendersBase benders_;
  double lambda_ = 0;
  double lambda_min_ = 0;
  double lambda_max_ = 1;
  double tau_ = 0.5;
};
