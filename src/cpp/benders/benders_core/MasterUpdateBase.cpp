#include "MasterUpdate.h"

MasterUpdateBase::MasterUpdateBase(pBendersBase benders, double lambda,
                                   double lambda_min, double lambda_max,
                                   double tau)
    : benders_(std::move(benders)),
      lambda_(lambda),
      lambda_min_(lambda_min),
      lambda_max_(lambda_max) {
  if (tau >= 0 && tau <= 1) {
    // TODO log
    tau_ = tau;
  }
}

void MasterUpdateBase::Update(const CRITERION &criterion) {
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
void MasterUpdateBase::UpdateConstraints() {
  if (auto row_index = benders_->MasterRowIndex(ADDITIONAL_ROW_NAME);
      row_index > -1) {
    benders_->MasterChangeRhs(row_index, lambda_);

  } else {
    auto master_variables = benders_->MasterVariables();
    auto invest_cost = benders_->BestIterationInvestCost();
    // ajouter la cont:
    auto newnz = master_variables.size();
    int newrows = 1;
    std::vector<char> rtype(newrows, 'L');
    std::vector<double> rhs(newrows, lambda_);
    std::vector<int> mclind(newnz);
    size_t mclindCnt_l(0);
    std::vector<double> matval(newnz);
    for (auto const &[name, cost] : invest_cost) {
      mclind[mclindCnt_l] = master_variables.at(name);
      matval[mclindCnt_l] = invest_cost.at(name);
      ++mclindCnt_l;
    }
    std::vector<int> matstart(newrows + 1);
    matstart[0] = 0;
    matstart[1] = newnz;
    benders_->MasterAddRows(rtype, rhs, {}, matstart, mclind, matval);
  }
}

// void MasterUpdateBase::AddCutsInMaster() {}