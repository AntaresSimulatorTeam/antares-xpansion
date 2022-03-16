#ifndef __ANALYSISFUNCTION_H__
#define __ANALYSISFUNCTION_H__
#include "SensitivityOutputData.h"
#include "multisolver_interface/SolverAbstract.h"
RawPbData solve_sensitivity_pb(const SolverAbstract::Ptr &sensitivity_problem);
#endif  //__ANALYSISFUNCTION_H__
