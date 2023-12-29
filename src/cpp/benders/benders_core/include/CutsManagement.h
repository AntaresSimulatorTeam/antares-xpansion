#pragma once
#include "BendersStructsDatas.h"

class ICutsManager {
 public:
  ICutsManager() = default;
  void virtual Save(const BendersCutsPerIteration& benders_cuts) = 0;
  virtual BendersCutsPerIteration Load() = 0;
};

class CutsManagerRunTime : public ICutsManager {
 public:
  void Save(const BendersCutsPerIteration& benders_cuts) override;
  BendersCutsPerIteration Load() override;

 private:
  BendersCutsPerIteration benders_cuts_;
};
