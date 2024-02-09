#include "CutsManagement.h"

void CutsManagerRunTime::Save(const BendersCutsPerIteration& benders_cuts) {
  benders_cuts_ = benders_cuts;
}

BendersCutsPerIteration CutsManagerRunTime::Load() { return benders_cuts_; }
