#include "CutsManagement.h"

CutManagerRunTime::Save(const BendersCuts& benders_cuts) {
  benders_cuts_ = benders_cuts;
}

BendersCuts CutManagerRunTime::Load() { return benders_cuts_; }
