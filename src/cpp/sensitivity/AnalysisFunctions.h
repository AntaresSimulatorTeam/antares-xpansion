#ifndef __ANALYSISFUNCTION_H__
#define __ANALYSISFUNCTION_H__
#include "ProblemModifierCapex.h"
#include "ProblemModifierProjection.h"
#include "SensitivityInputReader.h"
#include "SensitivityOutputData.h"
#include "multisolver_interface/SolverAbstract.h"
SolverAbstract::Ptr get_sensitivity_problem(
    const SensitivityInputData input_data, const std::string& candidate_name,
    SensitivityPbType type);
RawPbData solve_sensitivity_pb(SensitivityInputData input_data, const SolverAbstract::Ptr& sensitivity_problem);
#endif  //__ANALYSISFUNCTION_H__
