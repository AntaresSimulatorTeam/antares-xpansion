#include "MasterUpdate.h"

MasterUpdateBase::MasterUpdateBase(double lambda, double lambda_min,
                                   double lambda_max, double tau)
    : lambda_(lambda), lambda_min_(lambda_min), lambda_max_(lambda_max) {
  if (tau >= 0 && tau <= 1) {
    // TODO log
    tau_ = tau;
  }
}

void MasterUpdateBase::Update(const CRITERION& criterion,
                              pBendersBase benders) {
  switch (criterion) {
    case CRITERION::LESSER:
      // TODO best it or current data?
      lambda_max_ =
          std::min(lambda_max_, benders->GetBestIterationData().invest_cost);
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
  AddConstraints();
  AddCutsInMaster();
}
void MasterUpdateBase::AddConstraints() {}
void MasterUpdateBase::AddCutsInMaster() {}