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
  UpdateConstraints(benders_);
  // deplacer dans Benders
  // AddCutsInMaster();
}
void MasterUpdateBase::UpdateConstraints(pBendersBase benders) {
  // if (cnt exist) {
  //   // modification du 2nd membre = lambda
  // } else {
  //   // ajouter la cont:
  //   auto newnz = benders->;
  //   int newrows = 1;
  //   std::vector<char> rtype(newrows);
  //   std::vector<double> rhs(newrows, additionalConstraint_p.getRHS());
  //   std::vector<int> mindex(newnz);
  //   std::vector<double> matval(newnz);
  //   std::vector<int> matstart(newrows + 1);
  // }
}

// void MasterUpdateBase::AddCutsInMaster() {}