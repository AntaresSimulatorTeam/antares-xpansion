#pragma once
#include "BendersStructsDatas.h"

class ICutsManager {
 public:
  ICutsManager() = default;
  void virtual Save(const BendersCuts& benders_cuts) = 0;
  virtual BendersCuts Load() = 0;
};

class CutManagerRunTime : public ICutsManager {
 public:
  void Save(const BendersCuts& benders_cuts) override;
  BendersCuts Load() override;

 private:
  BendersCuts benders_cuts_;
};

class
