#pragma once
#include "BendersStructsDatas.h"

namespace Outerloop {

class ICutsManager {
 public:
  ICutsManager() = default;
  void virtual Save(const WorkerMasterDataVect& benders_cuts) = 0;
  virtual WorkerMasterDataVect Load() = 0;
  virtual ~ICutsManager() = default;
};

class CutsManagerRunTime : public ICutsManager {
 public:
  void Save(const WorkerMasterDataVect& benders_cuts) override;
  WorkerMasterDataVect Load() override;
  virtual ~CutsManagerRunTime() = default;

 private:
  WorkerMasterDataVect benders_cuts_;
};

}  // namespace Outerloop
