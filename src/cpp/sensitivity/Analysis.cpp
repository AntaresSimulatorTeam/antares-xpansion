#include "Analysis.h"

#include "antares-xpansion/sensitivity/ProblemModifierCapex.h"
#include "antares-xpansion/sensitivity/ProblemModifierProjection.h"
#include "antares-xpansion/sensitivity/SensitivityOutputData.h"

Analysis::Analysis(SensitivityInputData input_data, std::string candidate_name,
                   std::shared_ptr<SensitivityILogger> logger,
                   SensitivityPbType type)
    : input_data(std::move(input_data)),
      candidate_name(std::move(candidate_name)),
      logger(std::move(logger)),
      problem_type(type) {}

void Analysis::fill_single_pb_data(SinglePbData &pb_data,
                                   const RawPbData &raw_output) const {
  pb_data.objective = raw_output.obj_value;
  pb_data.solver_status = raw_output.status;

  for (auto const &kvp : input_data.name_to_id) {
    pb_data.candidates[kvp.first] = raw_output.solution[kvp.second];
  }
  pb_data.system_cost = get_system_cost(raw_output);
}
double Analysis::get_system_cost(const RawPbData &raw_output) const {
  int ncols = input_data.last_master->get_ncols();
  std::vector<double> master_obj(ncols);

  input_data.last_master->get_obj(master_obj.data(), 0, ncols - 1);

  double system_cost = 0;
  for (int col_id(0); col_id < ncols; col_id++) {
    system_cost += master_obj[col_id] * raw_output.solution[col_id];
  }
  return system_cost;
}

SinglePbData Analysis::run_optimization(SensitivityStudy::StudyType minimize) {
  sensitivity_pb_model->chg_obj_direction(static_cast<bool>(minimize));

  SinglePbData pb_data = {
      problem_type, get_string_from_SensitivityPbType(problem_type),
      candidate_name,
      minimize == SensitivityStudy::StudyType::MINIMIZE ? MIN_C : MAX_C};
  logger->log_begin_pb_resolution(pb_data);

  auto raw_output = solve_sensitivity_pb();

  fill_single_pb_data(pb_data, raw_output);
  logger->log_pb_solution(pb_data);

  return pb_data;
}
std::pair<SinglePbData, SinglePbData> Analysis::run() {
  get_sensitivity_problem();
  auto min_data = run_optimization(SensitivityStudy::StudyType::MINIMIZE);
  auto max_data = run_optimization(SensitivityStudy::StudyType::MAXIMIZE);
  return {min_data, max_data};
}

RawPbData Analysis::solve_sensitivity_pb() const {
  RawPbData raw_output;
  int ncols = sensitivity_pb_model->get_ncols();

  raw_output.obj_coeffs.resize(ncols);
  raw_output.solution.resize(ncols);

  sensitivity_pb_model->get_obj(raw_output.obj_coeffs.data(), 0, ncols - 1);

  if (sensitivity_pb_model->get_n_integer_vars() > 0) {
    raw_output.status = sensitivity_pb_model->solve_mip();
    sensitivity_pb_model->get_mip_sol(raw_output.solution.data());
    raw_output.obj_value = sensitivity_pb_model->get_mip_value();
  } else {
    raw_output.status = sensitivity_pb_model->solve_lp();
    sensitivity_pb_model->get_lp_sol(raw_output.solution.data(), nullptr,
                                     nullptr);
    raw_output.obj_value = sensitivity_pb_model->get_lp_value();
  }

  return raw_output;
}

void Analysis::get_sensitivity_problem() {
  unsigned int nb_candidates = input_data.name_to_id.size();

  switch (problem_type) {
    case SensitivityPbType::CAPEX: {
      ProblemModifierCapex pb_modifier(input_data.epsilon, input_data.best_ub,
                                       input_data.last_master);
      sensitivity_pb_model = pb_modifier.changeProblem(nb_candidates);
      break;
    }
    case SensitivityPbType::PROJECTION: {
      ProblemModifierProjection pb_modifier(
          input_data.epsilon, input_data.best_ub, input_data.last_master,
          input_data.name_to_id.at(candidate_name), candidate_name);
      sensitivity_pb_model = pb_modifier.changeProblem(nb_candidates);
      break;
    }

    default:
      std::cerr << "Unrecognized Sensitivity Problem type" << std::endl;
  }

  if (std::ifstream basis_file(input_data.basis_file_path); basis_file.good()) {
    sensitivity_pb_model->read_basis(input_data.basis_file_path);
  } else {
    logger->display_message("Warning: Basis file " +
                            input_data.basis_file_path.string() +
                            " could not be read.");
  }
}