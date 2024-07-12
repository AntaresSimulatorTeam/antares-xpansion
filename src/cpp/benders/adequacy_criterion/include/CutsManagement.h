#pragma once
#include "BendersStructsDatas.h"

namespace AdequacyCriterionSpace {

class ICutsManager {
 public:
  ICutsManager() = default;
  void virtual Save(const WorkerMasterDataVect& benders_cuts) = 0;
  virtual WorkerMasterDataVect Load() = 0;
};

class CutsManagerRunTime : public ICutsManager {
 public:
  void Save(const WorkerMasterDataVect& benders_cuts) override;
  WorkerMasterDataVect Load() override;

 private:
  WorkerMasterDataVect benders_cuts_;
};

}  // namespace AdequacyCriterionSpace
