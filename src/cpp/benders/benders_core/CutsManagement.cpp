#include "CutsManagement.h"

void CutsManagerRunTime::Save(const BendersCuts& benders_cuts) {
  benders_cuts_ = benders_cuts;
}

BendersCuts CutsManagerRunTime::Load() { return benders_cuts_; }
