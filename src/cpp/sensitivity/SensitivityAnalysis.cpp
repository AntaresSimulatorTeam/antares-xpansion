#include "SensitivityAnalysis.h"

SensitivityAnalysis::SensitivityAnalysis(double epsilon, double bestUb, const std::map<int, std::string> &idToName, SolverAbstract::Ptr lastMaster, std::shared_ptr<SensitivityWriter> writer) : _epsilon(epsilon), _best_ub(bestUb), _id_to_name(idToName), _last_master(lastMaster), _writer(writer), _pb_modifier(epsilon, bestUb)
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

SensitivityOutputData SensitivityAnalysis::get_output_data() const
{
	return _output_data;
}

void SensitivityAnalysis::init_output_data()
{
	_output_data.epsilon = _epsilon;
	_output_data.best_benders_cost = _best_ub;
	_output_data.solution_system_cost = 1e+20;
	_output_data.pb_objective = 1e+20;
	for (auto const &kvp : _id_to_name)
	{
		_output_data.candidates[kvp.second] = 1e+20;
	}
	_output_data.pb_status = SOLVER_STATUS::UNKNOWN;
}

// void SensitivityAnalysis::get_capex_optimal_solutions()
// {
// 	get_capex_min_solution();
// 	get_capex_max_solution();
// }

void SensitivityAnalysis::get_capex_min_solution()
{

	auto sensitivity_pb_model = _pb_modifier.changeProblem(_id_to_name, _last_master);
	// _sensitivity_pb_model = _pb_modifier.getProblem();

	auto solution = get_sensitivity_solution(sensitivity_pb_model);
	fill_output_data(solution);
}

// void SensitivityAnalysis::get_capex_max_solution() {

// 	auto pb_modifier = SensitivityPbModifier();
// 	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

// 	get_sensitivity_solution();
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

// 	get_sensitivity_solution();
// }

// void SensitivityAnalysis::get_candidate_upper_projection(int &candidateNum) {
// 	auto pb_modifier = SensitivityPbModifier();
// 	_sensitivity_pb_model = pb_modifier.changeProblem(_last_master_model);

// 	get_sensitivity_solution();
// }

std::vector<double> SensitivityAnalysis::get_sensitivity_solution(SolverAbstract::Ptr sensitivity_problem)
{

	std::vector<double> solution(sensitivity_problem->get_ncols());

	if (sensitivity_problem->get_n_integer_vars() > 0)
	{
		_output_data.pb_status = sensitivity_problem->solve_mip();
		sensitivity_problem->get_mip_sol(solution.data());
		_output_data.pb_objective = sensitivity_problem->get_mip_value();
	}
	else
	{
		_output_data.pb_status = sensitivity_problem->solve_lp();
		sensitivity_problem->get_lp_sol(solution.data(), nullptr, nullptr);
		_output_data.pb_objective = sensitivity_problem->get_lp_value();
	}
	return solution;
}

void SensitivityAnalysis::fill_output_data(const std::vector<double> &solution)
{
	int nb_candidates = _id_to_name.size();
	for (auto const &kvp : _id_to_name)
	{
		_output_data.candidates[kvp.second] = solution[kvp.first];
	}
	_output_data.solution_system_cost = _output_data.pb_objective + solution[nb_candidates];
}