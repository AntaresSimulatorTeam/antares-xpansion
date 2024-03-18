#pragma once
#include "OuterLoopCriterion.h"

class IMasterUpdate {
 public:
  virtual void Update(const CRITERION &criterion) = 0;
  virtual void Init() = 0;
};

class MasterUpdateBase : public IMasterUpdate {
 public:
  explicit MasterUpdateBase(pBendersBase benders, double lambda,
                            double lambda_min, double lambda_max, double tau);
  explicit MasterUpdateBase(pBendersBase benders, double lambda,
                            double lambda_min, double lambda_max, double tau,
                            const std::string &name);
  explicit MasterUpdateBase(pBendersBase benders, double tau,
                            const std::string &name);
  explicit MasterUpdateBase(pBendersBase benders, double tau);
  void Update(const CRITERION &criterion) override;
  void Init() override;

 private:
  void CheckTau(double tau);
  void SetLambdaMaxToMaxInvestmentCosts();
  void UpdateConstraints();
  void AddMinInvestConstraint();
  // rename min invest constraint
  std::string min_invest_constraint_name_ = "Min_Investment_Constraint";
  int additional_constraint_index_ = -1;
  pBendersBase benders_;
  double lambda_ = 0;
  double lambda_min_ = 0;
  double lambda_max_ = -1;
  // tau
  double dichotomy_weight_coeff_ = 0.5;
};
