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
	raw_output.solution.resize(sensitivity_problem->get_ncols());

	if (sensitivity_problem->get_n_integer_vars() > 0)
	{
		raw_output.status = sensitivity_problem->solve_mip();
		sensitivity_problem->get_mip_sol(raw_output.solution.data());
		raw_output.objective = sensitivity_problem->get_mip_value();
	}
	else
	{
		raw_output.status = sensitivity_problem->solve_lp();
		sensitivity_problem->get_lp_sol(raw_output.solution.data(), nullptr, nullptr);
		raw_output.objective = sensitivity_problem->get_lp_value();
	}

	return raw_output;
}

void SensitivityAnalysis::fill_output_data(const RawPbData &raw_output, const bool minimize)
{
	SinglePbData pb_data;
	
	pb_data.pb_type = sensitivity_string_pb_type[_sensitivity_pb_type];
	pb_data.opt_dir = minimize ? "min" : "max";
	pb_data.objective = raw_output.objective;
	pb_data.status = raw_output.status;

	int nb_candidates = _id_to_name.size();
	for (auto const &kvp : _id_to_name)
	{
		pb_data.candidates[kvp.second] = raw_output.solution[kvp.first];
	}
	pb_data.system_cost = pb_data.objective + raw_output.solution[nb_candidates];

	_output_data.pbs_data.push_back(pb_data);
}