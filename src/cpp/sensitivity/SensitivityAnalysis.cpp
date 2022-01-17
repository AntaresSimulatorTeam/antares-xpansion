#include "SensitivityAnalysis.h"

SensitivityAnalysis::SensitivityAnalysis(std::shared_ptr<SolverAbstract> &solverModel, SensitivityPbModifier &pbModifier, SensitivityLogger &logger, SensitivityWriter writer) : _solver_model(solverModel), _pb_modifier(pbModifier), _logger(logger), _writer(writer) {}

void SensitivityAnalysis::launch() 
{
    _solver_model = _pb_modifier.changeProblem(_solver_model);
    
    std::vector<double> ptr(_solver_model->get_ncols());

	if (_solver_model->get_n_integer_vars() > 0) {
		_solver_model->get_mip_sol(ptr.data());
	}
	else {
		_solver_model->get_lp_sol(ptr.data(), NULL, NULL);
	}
}