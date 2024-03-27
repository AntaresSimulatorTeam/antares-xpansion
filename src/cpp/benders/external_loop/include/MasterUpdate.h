#pragma once
#include "OuterLoopCriterion.h"
#include "common.h"

class IMasterUpdate {
 public:
  virtual bool Update(double lambda_min, double lambda_max) = 0;
  virtual void Init() = 0;
  virtual double Rhs() const = 0;
};

class MasterUpdateBase : public IMasterUpdate {
 public:
  explicit MasterUpdateBase(pBendersBase benders, double tau,
                            double epsilon_lambda,
                            const ExternalLoopOptions &options);
  explicit MasterUpdateBase(pBendersBase benders, double tau,
                            const std::string &name, double epsilon_lambda,
                            const ExternalLoopOptions &options);
  bool Update(double lambda_min, double lambda_max) override;
  void Init() override;
  double Rhs() const override;

 private:
  void CheckTau(double tau);
  void UpdateConstraints();
  void AddMinInvestConstraint();
  // rename min invest constraint
  std::string min_invest_constraint_name_ = "Min_Investment_Constraint";
  int additional_constraint_index_ = -1;
  pBendersBase benders_;
  double lambda_ = 0;
  // tau
  double dichotomy_weight_coeff_ = 0.5;
  double outer_loop_stopping_threshold_ = 1e-1;
  bool stop_update_ = true;
};
