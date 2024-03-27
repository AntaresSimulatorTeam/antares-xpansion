#include "MasterUpdate.h"

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double tau,
                                   double epsilon_lambda,
                                   const ExternalLoopOptions &options)
    : benders_(std::move(benders)),
      lambda_(0),
      outer_loop_stopping_threshold_(epsilon_lambda) {
  CheckTau(tau);
}

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double tau,
                                   const std::string &name,
                                   double epsilon_lambda,
                                   const ExternalLoopOptions &options)
    : MasterUpdateBase(benders, tau, epsilon_lambda, options) {
  min_invest_constraint_name_ = name;
}

void MasterUpdateBase::CheckTau(double tau) {
  if (tau >= 0 && tau <= 1) {
    // TODO log
    dichotomy_weight_coeff_ = tau;
  }
}

void MasterUpdateBase::Init() {
  // check lambda_max
  // if (lambda_max <= 0 || lambda_max < lambda_min) {
  //   // TODO log
  //   SetLambdaMaxToMaxInvestmentCosts();
  // }
}

bool MasterUpdateBase::Update(double lambda_min, double lambda_max) {
  // if (is_criterion_high) {
  //   lambda_min = lambda_;
  // } else {
  //   lambda_max =
  //       std::min(lambda_max, benders_->GetBestIterationData().invest_cost);
  // }
  // WorkerMasterData workerMasterData = benders_->BestIterationWorkerMaster();
  // double invest_cost = workerMasterData._invest_cost;
  // double overall_cost = invest_cost + workerMasterData._operational_cost;
  // if (outerLoopBiLevel_.Update_bilevel_data_if_feasible(
  //         workerMasterData._cut_trace,
  //         benders_
  //             ->GetOuterLoopCriterionAtBestBenders() /*/!\ must be at best
  //             it*/,
  //         overall_cost)) {
  //   lambda_max = std::min(lambda_max, invest_cost);
  // } else {
  //   lambda_min = lambda_;
  // }

  stop_update_ =
      std::abs(lambda_max - lambda_min) < outer_loop_stopping_threshold_;
  if (!stop_update_) {
    lambda_ = dichotomy_weight_coeff_ * lambda_max +
              (1 - dichotomy_weight_coeff_) * lambda_min;
    UpdateConstraints();
  }
  return stop_update_;
}

void MasterUpdateBase::UpdateConstraints() {
  if (!benders_->MasterIsEmpty() && additional_constraint_index_ > -1) {
    benders_->MasterChangeRhs(additional_constraint_index_, lambda_);

  } else {
    AddMinInvestConstraint();
  }
}

/**
 * Add the new constraint in benders main problem
 * /!\ to be called once
 */
void MasterUpdateBase::AddMinInvestConstraint() {
  auto master_variables = benders_->MasterVariables();
  const auto obj_coeff = benders_->MasterObjectiveFunctionCoeffs();
  auto newnz = master_variables.size();
  int newrows = 1;
  std::vector<char> rtype(newrows, 'G');
  std::vector<double> rhs(newrows, lambda_);
  std::vector<int> mclind(newnz);

  size_t mclindCnt_l(0);
  std::vector<double> matval(newnz);
  for (auto const &[name, var_id] : master_variables) {
    mclind[mclindCnt_l] = var_id;
    matval[mclindCnt_l] = obj_coeff.at(var_id);
    ++mclindCnt_l;
  }
  std::vector<int> matstart(newrows + 1);
  matstart[0] = 0;
  matstart[1] = newnz;
  if (!min_invest_constraint_name_.empty()) {
    std::vector<std::string> row_names(newrows, min_invest_constraint_name_);

    benders_->MasterAddRows(rtype, rhs, {}, matstart, mclind, matval,
                            row_names);
  } else {
    benders_->MasterAddRows(rtype, rhs, {}, matstart, mclind, matval);
  }
  additional_constraint_index_ = benders_->MasterGetnrows() - 1;
}

double MasterUpdateBase::Rhs() const { return lambda_; }