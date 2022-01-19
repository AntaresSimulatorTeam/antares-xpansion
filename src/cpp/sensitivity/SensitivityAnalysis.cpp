#include "SensitivityAnalysis.h"
#include "SensitivityPbModifier.h"

SensitivityAnalysis::SensitivityAnalysis(std::shared_ptr<SolverAbstract> &lastMasterModel, std::shared_ptr<SensitivityWriter> writer) : _last_master_model(lastMasterModel), _writer(writer)
{
	_sensitivity_pb_model = NULL;
}

void SensitivityAnalysis::launch()
{
	// get_capex_optimal_solutions();
	// get_candidates_projection();
	SensitivityOutputData output_data = get_capex_min_solution();
	_writer->end_writing(output_data);
}

// void SensitivityAnalysis::get_capex_optimal_solutions()
// {
// 	get_capex_min_solution();
// 	get_capex_max_solution();
// }

SensitivityOutputData SensitivityAnalysis::get_capex_min_solution()
{

	auto pb_modifier = SensitivityPbModifier();
	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

	return solve_sensitivity_pb();
}

// void SensitivityAnalysis::get_capex_max_solution() {

// 	auto pb_modifier = SensitivityPbModifier();
// 	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

// 	solve_sensitivity_pb();
// }

// void SensitivityAnalysis::get_candidates_projection()
// {
// 	for (int candidateNum(0); candidateNum < _last_master_model->get_ncols(); candidateNum++)
// 	{
// 		get_candidate_lower_projection(candidateNum);
// 		get_candidate_upper_projection(candidateNum);
// 	}
// }

// void SensitivityAnalysis::get_candidate_lower_projection(int &candidateNum) {
// 	auto pb_modifier = SensitivityPbModifier();
// 	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

// 	solve_sensitivity_pb();
// }

// void SensitivityAnalysis::get_candidate_upper_projection(int &candidateNum) {
// 	auto pb_modifier = SensitivityPbModifier();
// 	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

// 	solve_sensitivity_pb();
// }

SensitivityOutputData SensitivityAnalysis::solve_sensitivity_pb()
{

	std::vector<double> ptr(_sensitivity_pb_model->get_ncols());

	if (_sensitivity_pb_model->get_n_integer_vars() > 0)
	{
		_sensitivity_pb_model->get_mip_sol(ptr.data());
	}
	else
	{
		_sensitivity_pb_model->get_lp_sol(ptr.data(), NULL, NULL);
	}
}