#include "MasterUpdate.h"

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double tau)
    : benders_(std::move(benders)), lambda_(0), lambda_min_(0) {
  CheckTau(tau);
}

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double tau,
                                   const std::string &name)
    : MasterUpdateBase(benders, tau) {
  additional_constraint_name_ = name;
}
MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double lambda,
                                   double lambda_min, double lambda_max,
                                   double tau)
    : benders_(std::move(benders)),
      lambda_(lambda),
      lambda_min_(lambda_min),
      lambda_max_(lambda_max) {
  CheckTau(tau);
}

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double lambda,
                                   double lambda_min, double lambda_max,
                                   double tau, const std::string &name)
    : MasterUpdateBase(benders, lambda, lambda_min, lambda_max, tau) {
  additional_constraint_name_ = name;
}

void MasterUpdateBase::CheckTau(double tau) {
  if (tau >= 0 && tau <= 1) {
    // TODO log
    tau_ = tau;
  }
}
void MasterUpdateBase::SetLambdaMaxToMaxInvestmentCosts() {
  const auto &obj = benders_->ObjectiveFunctionCoeffs();
  const auto max_invest =
      benders_->BestIterationWorkerMaster().get_max_invest();
  lambda_max_ = 0;
  for (const auto &[var_name, var_id] : benders_->MasterVariables()) {
    lambda_max_ += obj[var_id] * max_invest.at(var_name);
  }
  // lambda_max_ = 126000 * 300 + 60000 * 2000 + 55400 * 1000 + 60000 * 1000;
}
void MasterUpdateBase::Update(const CRITERION &criterion) {
  // check lambda_max_
  // whar abour lambda_min_?
  if (lambda_max_ <= 0 || lambda_max_ < lambda_min_) {
    // TODO log
    SetLambdaMaxToMaxInvestmentCosts();
    return;
  }
  switch (criterion) {
    case CRITERION::LESSER:
      // TODO best it or current data?
      lambda_max_ =
          std::min(lambda_max_, benders_->GetBestIterationData().invest_cost);
      break;
    case CRITERION::GREATER:
      lambda_min_ = lambda_;
      break;

    default:
      // TODO what to do here?
      return;
      //   break;
  }
  lambda_ = tau_ * lambda_max_ + (1 - tau_) * lambda_min_;
  // benders->mas
  UpdateConstraints();
  // deplacer dans Benders
  // AddCutsInMaster();
}

// bool MasterUpdateBase::IsConstraintInMasterProblem(int &row_index) const {
//   if (!additional_constraint_name_.empty()) {
//     row_index = benders_->MasterRowIndex(additional_constraint_name_);
//   }
//   return row_index > -1;
// }
void MasterUpdateBase::UpdateConstraints() {
  if (!benders_->MasterIsEmpty() && additional_constraint_index_ > -1) {
    benders_->MasterChangeRhs(additional_constraint_index_, lambda_);

  } else {
    // benders_->ResetMasterFromLastIteration();
    auto master_variables = benders_->MasterVariables();
    const auto obj_coeff = benders_->ObjectiveFunctionCoeffs();
    // ajouter la cont:
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
    if (!additional_constraint_name_.empty()) {
      std::vector<std::string> row_names(newrows, additional_constraint_name_);

      benders_->MasterAddRows(rtype, rhs, {}, matstart, mclind, matval,
                              row_names);
    } else {
      benders_->MasterAddRows(rtype, rhs, {}, matstart, mclind, matval);
    }
    additional_constraint_index_ = benders_->MasterGetnrows() - 1;
  }
}

// void MasterUpdateBase::AddCutsInMaster() {}