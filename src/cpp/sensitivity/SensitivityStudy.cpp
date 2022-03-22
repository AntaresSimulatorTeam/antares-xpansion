#include "SensitivityStudy.h"

#include <utility>

#include "Analysis.h"

SensitivityStudy::SensitivityStudy(const SensitivityInputData &input_data,
                                   std::shared_ptr<SensitivityILogger> logger,
                                   std::shared_ptr<SensitivityWriter> writer)
    : logger(std::move(logger)),
      writer(std::move(writer)),
      input_data(input_data) {
  init_output_data();
}

void SensitivityStudy::launch() {
  logger->log_at_start(output_data);

  if (input_data.capex) {
    run_capex_analysis();
  }
  if (!input_data.projection.empty()) {
    run_projection_analysis();
  }
  if (!input_data.capex && input_data.projection.empty()) {
    logger->display_message("Study is empty. No capex or projection provided.");
  } else {
    logger->log_summary(output_data);
  }
  writer->end_writing(output_data);
  logger->log_at_ending();
}

SensitivityOutputData SensitivityStudy::get_output_data() const {
  return output_data;
}

void SensitivityStudy::init_output_data() {
  output_data.epsilon = input_data.epsilon;
  output_data.best_benders_cost = input_data.best_ub;
  output_data.candidates_bounds = get_candidates_bounds();
  output_data.pbs_data = {};
}

std::string &rtrim(std::string &s) {
  auto it = std::find_if(s.rbegin(), s.rend(), [](unsigned char c) {
    return !std::isspace<char>(c, std::locale::classic());
  });
  s.erase(it.base(), s.end());
  return s;
}

std::map<std::string, std::pair<double, double>>
SensitivityStudy::get_candidates_bounds() {
  std::map<std::string, std::pair<double, double>> candidates_bounds;

  unsigned int nb_candidates = input_data.name_to_id.size();
  std::vector<double> lb_buffer(nb_candidates);
  std::vector<double> ub_buffer(nb_candidates);
  std::vector<std::string> candidates_name =
      input_data.last_master->get_col_names(0, nb_candidates - 1);

  for (int i = 0; i < candidates_name.size(); i++) {
    candidates_name[i] = rtrim(candidates_name[i]);
  }

  input_data.last_master->get_lb(lb_buffer.data(), 0, nb_candidates - 1);
  input_data.last_master->get_ub(ub_buffer.data(), 0, nb_candidates - 1);

  for (int i = 0; i < nb_candidates; i++) {
    candidates_bounds[candidates_name[i]] = {lb_buffer[i], ub_buffer[i]};
  }
  return candidates_bounds;
}

void SensitivityStudy::run_capex_analysis() {
  logger->display_message("Capex analysis in progress");
  Analysis capex_analysis(input_data, "", logger, SensitivityPbType::CAPEX);
  std::pair<SinglePbData, SinglePbData> capex_data = capex_analysis.run();

  output_data.pbs_data.push_back(capex_data.first);
  output_data.pbs_data.push_back(capex_data.second);
}

void SensitivityStudy::run_projection_analysis() {
  logger->display_message("Projection analysis in progress");

  for (auto const &candidate_name : input_data.projection) {
    if (input_data.name_to_id.find(candidate_name) !=
        input_data.name_to_id.end()) {
      Analysis projection_analysis(input_data, candidate_name, logger,
                                   SensitivityPbType::PROJECTION);
      auto problem_data = projection_analysis.run();
      output_data.pbs_data.push_back(problem_data.first);
      output_data.pbs_data.push_back(problem_data.second);
    } else {
      logger->display_message("Warning : " + candidate_name +
                              " ignored as it has not been found in the list "
                              "of investment candidates");
    }
  }
}