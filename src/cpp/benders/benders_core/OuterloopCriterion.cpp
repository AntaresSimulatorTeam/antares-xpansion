#pragma once
#include "OuterLoopCriterion.h"

OuterloopCriterionLOL::OuterloopCriterionLOL(const BendersCuts& benders_cuts,
                                             double threshold, double epsilon)
    : benders_cuts_(benders_cuts), threshold_(threshold), epsilon_(epsilon) {}