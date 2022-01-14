#include "SensitivityAnalysis.h"

SensitivityAnalysis::SensitivityAnalysis(std::shared_ptr<SolverAbstract> &math_problem, SensitivityLogger &logger, SensitivityWriter writer) : _math_problem(math_problem), _logger(logger), _writer(writer) {}