#include "SensitivityAnalysis.h"

#include "PbModifierCapex.h"
#include "PbModifierProjection.h"

const bool SensitivityAnalysis::MINIMIZE = true;
const bool SensitivityAnalysis::MAXIMIZE = false;
const std::vector<std::string> SensitivityAnalysis::sensitivity_string_pb_type{
    CAPEX_C, PROJECTION_C};

SensitivityAnalysis::SensitivityAnalysis(
    const SensitivityInputData &input_data,
    std::shared_ptr<SensitivityILogger> logger,
    std::shared_ptr<SensitivityWriter> writer)
    : _epsilon(input_data.epsilon),
      _best_ub(input_data.best_ub),
      _capex(input_data.capex),
      _projection(input_data.projection),
      _name_to_id(input_data.name_to_id),
      _last_master(input_data.last_master),
      _logger(logger),
      _writer(writer) {
  init_output_data();
}

void SensitivityAnalysis::launch() {
  _logger->log_at_start(_output_data);

  if (_capex) {
    get_capex_solutions();
  }
  if (!_projection.empty()) {
    get_candidates_projection();
  }
  _logger->log_summary(_output_data);
  _writer->end_writing(_output_data);
  _logger->log_at_ending();
}

SensitivityOutputData SensitivityAnalysis::get_output_data() const {
  return _output_data;
}

void SensitivityAnalysis::init_output_data() {
  _output_data.epsilon = _epsilon;
  _output_data.best_benders_cost = _best_ub;
  _output_data.pbs_data = {};
}

void SensitivityAnalysis::get_capex_solutions() {
  _sensitivity_pb_type = SensitivityPbType::CAPEX;
  _pb_modifier = std::make_shared<PbModifierCapex>(_epsilon, _best_ub);
  run_analysis();
}

void SensitivityAnalysis::get_candidates_projection() {
  _sensitivity_pb_type = SensitivityPbType::PROJECTION;

  for (auto const &candidate_name : _projection) {
    if (_name_to_id.find(candidate_name) != _name_to_id.end()) {
      _pb_modifier = std::make_shared<PbModifierProjection>(
          _epsilon, _best_ub, _name_to_id[candidate_name], candidate_name);
      run_analysis();
    } else {
      // TODO : Improve this ?
      _logger->display_message("Warning : " + candidate_name +
                               "ignored as it has not been found in the list "
                               "of investment candidates");
    }
  }
}

void SensitivityAnalysis::run_analysis() {
  int nb_candidates = _name_to_id.size();
  auto sensitivity_pb_model =
      _pb_modifier->changeProblem(nb_candidates, _last_master);

  run_optimization(sensitivity_pb_model, MINIMIZE);
  run_optimization(sensitivity_pb_model, MAXIMIZE);
}

void SensitivityAnalysis::run_optimization(
    const SolverAbstract::Ptr &sensitivity_model, const bool minimize) {
  sensitivity_model->chg_obj_direction(minimize);

  SinglePbData pb_data = init_single_pb_data(minimize);
  _logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb(sensitivity_model);

  fill_single_pb_data(pb_data, raw_output, minimize);
  _logger->log_pb_solution(pb_data);

  _output_data.pbs_data.push_back(pb_data);
}

SinglePbData SensitivityAnalysis::init_single_pb_data(const bool minimize) {
  std::string candidate_name = "";
  std::string str_pb_type =
      sensitivity_string_pb_type[static_cast<int>(_sensitivity_pb_type)];
  if (_sensitivity_pb_type == SensitivityPbType::PROJECTION) {
    // Add a check for the success of the cast
    auto *pb_modifier =
        dynamic_cast<PbModifierProjection *>(_pb_modifier.get());
    candidate_name = pb_modifier->get_candidate_name();
  }
  std::string opt_dir = minimize ? MIN_C : MAX_C;
  return SinglePbData(_sensitivity_pb_type, str_pb_type, candidate_name,
                      opt_dir);
}

RawPbData SensitivityAnalysis::solve_sensitivity_pb(
    SolverAbstract::Ptr sensitivity_problem) const {
  RawPbData raw_output;
  int ncols = sensitivity_problem->get_ncols();

  raw_output.obj_coeffs.resize(ncols);
  raw_output.solution.resize(ncols);

  sensitivity_problem->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

  if (sensitivity_problem->get_n_integer_vars() > 0) {
    raw_output.status = sensitivity_problem->solve_mip();
    sensitivity_problem->get_mip_sol(raw_output.solution.data());
    raw_output.obj_value = sensitivity_problem->get_mip_value();
  } else {
    raw_output.status = sensitivity_problem->solve_lp();
    sensitivity_problem->get_lp_sol(raw_output.solution.data(), nullptr,
                                    nullptr);
    raw_output.obj_value = sensitivity_problem->get_lp_value();
  }

  return raw_output;
}

void SensitivityAnalysis::fill_single_pb_data(SinglePbData &pb_data,
                                              const RawPbData &raw_output,
                                              const bool minimize) {
  pb_data.objective = raw_output.obj_value;
  pb_data.status = raw_output.status;

  for (auto const &kvp : _name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}

double SensitivityAnalysis::get_system_cost(const RawPbData &raw_output) const {
  int ncols = _last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  _last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}