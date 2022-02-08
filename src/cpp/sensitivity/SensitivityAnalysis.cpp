#include "SensitivityAnalysis.h"
#include "PbModifierCapex.h"
#include "PbModifierProjection.h"

const bool SensitivityAnalysis::MINIMIZE = true;
const bool SensitivityAnalysis::MAXIMIZE = false;
const std::vector<std::string> SensitivityAnalysis::sensitivity_string_pb_type{CAPEX_C, PROJECTION_C};

SensitivityAnalysis::SensitivityAnalysis(double epsilon, double best_ub, const std::map<int, std::string> &id_to_name, SolverAbstract::Ptr last_master, std::shared_ptr<SensitivityWriter> writer) : _epsilon(epsilon), _best_ub(best_ub), _id_to_name(id_to_name), _last_master(last_master), _writer(writer)
{
	init_output_data();
}

void SensitivityAnalysis::launch()
{
	get_capex_solutions();
	get_candidates_projection();
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
	_output_data.pbs_data = {};
}

void SensitivityAnalysis::get_capex_solutions()
{
	_sensitivity_pb_type = CAPEX;
	_pb_modifier = std::make_shared<PbModifierCapex>(_epsilon, _best_ub);
	run_analysis();
}

void SensitivityAnalysis::get_candidates_projection()
{
	_sensitivity_pb_type = PROJECTION;
	for (auto const &kvp : _id_to_name)
	{
		_pb_modifier = std::make_shared<PbModifierProjection>(_epsilon, _best_ub, kvp.first);
		run_analysis();
	}
}

void SensitivityAnalysis::run_analysis()
{
	int nb_candidates = _id_to_name.size();
	auto sensitivity_pb_model = _pb_modifier->changeProblem(nb_candidates, _last_master);

	run_optimization(sensitivity_pb_model, MINIMIZE);
	run_optimization(sensitivity_pb_model, MAXIMIZE);
}

void SensitivityAnalysis::run_optimization(const SolverAbstract::Ptr &sensitivity_model, const bool minimize)
{
	sensitivity_model->chg_obj_sense(minimize);
	auto raw_output = solve_sensitivity_pb(sensitivity_model);
	fill_output_data(raw_output, minimize);
}

RawPbData SensitivityAnalysis::solve_sensitivity_pb(SolverAbstract::Ptr sensitivity_problem)
{
	RawPbData raw_output;
	int ncols = sensitivity_problem->get_ncols();

	raw_output.obj_coeffs.resize(ncols);
	raw_output.solution.resize(ncols);

	sensitivity_problem->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

	if (sensitivity_problem->get_n_integer_vars() > 0)
	{
		raw_output.status = sensitivity_problem->solve_mip();
		sensitivity_problem->get_mip_sol(raw_output.solution.data());
		raw_output.obj_value = sensitivity_problem->get_mip_value();
	}
	else
	{
		raw_output.status = sensitivity_problem->solve_lp();
		sensitivity_problem->get_lp_sol(raw_output.solution.data(), nullptr, nullptr);
		raw_output.obj_value = sensitivity_problem->get_lp_value();
	}

	return raw_output;
}

void SensitivityAnalysis::fill_output_data(const RawPbData &raw_output, const bool minimize)
{
	SinglePbData pb_data;

	pb_data.opt_dir = minimize ? MIN_C : MAX_C;
	pb_data.objective = raw_output.obj_value;
	pb_data.status = raw_output.status;
	pb_data.pb_type = sensitivity_string_pb_type[_sensitivity_pb_type];

	if (_sensitivity_pb_type == PROJECTION)
	{
		pb_data.pb_type += " " + get_projection_pb_candidate_name(raw_output);
	}

	for (auto const &kvp : _id_to_name)
	{
		pb_data.candidates[kvp.second] = raw_output.solution[kvp.first];
	}
	pb_data.system_cost = get_system_cost(raw_output);

	_output_data.pbs_data.push_back(pb_data);
}

double SensitivityAnalysis::get_system_cost(const RawPbData &raw_output)
{
	int ncols = _last_master->get_ncols();
	std::vector<double> master_obj(ncols);

	_last_master->get_obj(master_obj.data(), 0, ncols - 1);

	double system_cost = 0;
	for (int col_id(0); col_id < ncols; col_id++)
	{
		system_cost += master_obj[col_id] * raw_output.solution[col_id];
	}
	return system_cost;
}

std::string SensitivityAnalysis::get_projection_pb_candidate_name(const RawPbData &raw_output)
{
	std::string candidate_name = "";
	
	auto obj_it = std::find(raw_output.obj_coeffs.begin(), raw_output.obj_coeffs.end(), 1);

	if (obj_it != raw_output.obj_coeffs.end())
	{
		candidate_name = _id_to_name[std::distance(raw_output.obj_coeffs.begin(), obj_it)];
	}
	
	return candidate_name;
}