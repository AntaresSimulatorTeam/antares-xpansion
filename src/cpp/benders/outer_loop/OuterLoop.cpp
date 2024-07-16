#include "OuterLoop.h"

#include "LoggerUtils.h"
using namespace Outerloop;

IOuterLoop::IOuterLoop(std::shared_ptr<IMasterUpdate> master_updater,
                       std::shared_ptr<ICutsManager> cuts_manager)
    : master_updater_(std::move(master_updater)),
      cuts_manager_(std::move(cuts_manager)) {}
