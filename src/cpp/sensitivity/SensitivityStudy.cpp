#include "SensitivityStudy.h"

#include "CapexAnalysis.h"
#include "PbModifierCapex.h"
#include "PbModifierProjection.h"

const bool SensitivityStudy::MINIMIZE = true;
const bool SensitivityStudy::MAXIMIZE = false;
const std::vector<std::string> SensitivityStudy::sensitivity_string_pb_type{
    CAPEX_C, PROJECTION_C};

SensitivityStudy::SensitivityStudy(
    const SensitivityInputData &input_data,
    std::shared_ptr<SensitivityILogger> logger,
    std::shared_ptr<SensitivityWriter> writer)
    : _epsilon(input_data.epsilon),
      _best_ub(input_data.best_ub),
      _capex(input_data.capex),
      _projection(input_data.projection),
      _name_to_id(input_data.name_to_id),
      _last_master(input_data.last_master),
      logger(logger),
      writer(writer),
      input_data(input_data){
  init_output_data();
}

void SensitivityStudy::launch() {
  logger->log_at_start(_output_data);

  if (_capex) {
    run_capex_analysis();
  }
  if (!_projection.empty()) {
    get_candidates_projection();
  }
  logger->log_summary(_output_data);
  writer->end_writing(_output_data);
  logger->log_at_ending();
}

SensitivityOutputData SensitivityStudy::get_output_data() const {
  return _output_data;
}

void SensitivityStudy::init_output_data() {
  _output_data.epsilon = _epsilon;
  _output_data.best_benders_cost = _best_ub;
  _output_data.pbs_data = {};
}

void SensitivityStudy::run_capex_analysis() {
  CapexAnalysis capex_analysis(input_data, logger);
  std::pair<SinglePbData, SinglePbData> capex_data = capex_analysis.run();

  _output_data.pbs_data.push_back(capex_data.first);
  _output_data.pbs_data.push_back(capex_data.second);
}

void SensitivityStudy::get_candidates_projection() {
  _sensitivity_pb_type = SensitivityPbType::PROJECTION;

  for (auto const &candidate_name : _projection) {
    if (_name_to_id.find(candidate_name) != _name_to_id.end()) {
      _pb_modifier = std::make_shared<PbModifierProjection>(
          _epsilon, _best_ub, _last_master, _name_to_id[candidate_name], candidate_name);
      run_analysis();
    } else {
      // TODO : Improve this ?
      logger->display_message("Warning : " + candidate_name +
                               "ignored as it has not been found in the list "
                               "of investment candidates");
    }
  }
}

void SensitivityStudy::run_analysis() {
  int nb_candidates = _name_to_id.size();
  auto sensitivity_pb_model =
      _pb_modifier->changeProblem(nb_candidates);

  run_optimization(sensitivity_pb_model, MINIMIZE);
  run_optimization(sensitivity_pb_model, MAXIMIZE);
}

void SensitivityStudy::run_optimization(
    const SolverAbstract::Ptr &sensitivity_model, const bool minimize) {
  sensitivity_model->chg_obj_direction(minimize);

  SinglePbData pb_data = init_single_pb_data(minimize);
  logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb(sensitivity_model);

  fill_single_pb_data(pb_data, raw_output);
  logger->log_pb_solution(pb_data);

  _output_data.pbs_data.push_back(pb_data);
}

SinglePbData SensitivityStudy::init_single_pb_data(
    const bool minimize) const {
  std::string candidate_name = "";
  std::string str_pb_type =
      sensitivity_string_pb_type[static_cast<int>(_sensitivity_pb_type)];
  if (_sensitivity_pb_type == SensitivityPbType::PROJECTION) {
    // Add a check for the success of the cast
    const auto *pb_modifier =
        dynamic_cast<PbModifierProjection *>(_pb_modifier.get());
    candidate_name = pb_modifier->get_candidate_name();
  }
  std::string opt_dir = minimize ? MIN_C : MAX_C;
  return SinglePbData(_sensitivity_pb_type, str_pb_type, candidate_name,
                      opt_dir);
}

RawPbData SensitivityStudy::solve_sensitivity_pb(
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

void SensitivityStudy::fill_single_pb_data(
    SinglePbData &pb_data, const RawPbData &raw_output) const {
  pb_data.objective = raw_output.obj_value;
  pb_data.solver_status = raw_output.status;

  for (auto const &kvp : _name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}

double SensitivityStudy::get_system_cost(const RawPbData &raw_output) const {
  int ncols = _last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  _last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}