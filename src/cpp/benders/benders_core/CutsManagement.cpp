#include "antares-xpansion/benders/benders_core/CutsManagement.h"

using namespace Outerloop;

void CutsManagerRunTime::Save(const WorkerMasterDataVect& benders_cuts) {
  benders_cuts_ = benders_cuts;
}

WorkerMasterDataVect CutsManagerRunTime::Load() { return benders_cuts_; }
