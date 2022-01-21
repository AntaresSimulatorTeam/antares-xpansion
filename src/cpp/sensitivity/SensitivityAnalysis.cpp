#include "SensitivityAnalysis.h"

SensitivityAnalysis::SensitivityAnalysis(double epsilon, std::shared_ptr<SolverAbstract> lastMasterModel, std::shared_ptr<BendersData> bendersData, std::shared_ptr<SensitivityWriter> writer) : _epsilon(epsilon), _benders_data(bendersData), _sensitivity_pb_model(nullptr), _writer(writer), _pb_modifier(epsilon, bendersData, lastMasterModel)
{
	init_output_data();
}

void SensitivityAnalysis::launch()
{

	// get_capex_optimal_solutions();
	// get_candidates_projection();
	get_capex_min_solution();
	_writer->end_writing(_output_data);
}

void SensitivityAnalysis::init_output_data()
{
	_output_data.epsilon = _epsilon;
	_output_data.best_benders_cost = _benders_data->best_ub;
	_output_data.sensitivity_solution_overall_cost = 1e+20;
	_output_data.sensitivity_pb_objective = 1e+20;
	_output_data.sensitivity_candidates = _benders_data->x0;
	for (auto &candidateNameValue : _output_data.sensitivity_candidates)
	{
		candidateNameValue.second = 1e+20;
	}
	_output_data.sensitivity_pb_status = SOLVER_STATUS::UNKNOWN;
}

// void SensitivityAnalysis::get_capex_optimal_solutions()
// {
// 	get_capex_min_solution();
// 	get_capex_max_solution();
// }

void SensitivityAnalysis::get_capex_min_solution()
{

	_pb_modifier.changeProblem();
	_sensitivity_pb_model = _pb_modifier.getProblem();

	solve_sensitivity_pb();
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

void SensitivityAnalysis::solve_sensitivity_pb()
{

	std::vector<double> ptr(_sensitivity_pb_model->get_ncols());

	if (_sensitivity_pb_model->get_n_integer_vars() > 0)
	{
		_output_data.sensitivity_pb_status = _sensitivity_pb_model->solve_mip();
		_sensitivity_pb_model->get_mip_sol(ptr.data());
	}
	else
	{
		_output_data.sensitivity_pb_status = _sensitivity_pb_model->solve_lp();
		_sensitivity_pb_model->get_lp_sol(ptr.data(), NULL, NULL);
	}

	for (int idCandidate(0); idCandidate < _output_data.sensitivity_candidates.size(); idCandidate++)
	{
		
	}
}