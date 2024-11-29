#include <execution>
#include <tbb/tbb.h>


#include "antares-xpansion/sensitivity/SensitivityStudy.h"

#include <mutex>
#include <numeric>
#include <utility>

#include "Analysis.h"

SensitivityStudy::SensitivityStudy(SensitivityInputData input_data,
                                   std::shared_ptr<SensitivityILogger> logger,
                                   std::shared_ptr<SensitivityWriter> writer)
    : logger(std::move(logger)),
      writer(std::move(writer)),
      input_data(std::move(input_data)) {}

void SensitivityStudy::launch() {
  logger->log_at_start(input_data);
  if (input_data.capex) {
    run_capex_analysis();
  }
  if (!input_data.projection.empty()) {
    run_projection_analysis();
  }
  if (!input_data.capex && input_data.projection.empty()) {
    logger->display_message("Study is empty. No capex or projection provided.");
  } else {
    logger->log_summary(input_data, output_data);
  }
  writer->end_writing(input_data, output_data);
  logger->log_at_ending();
}

std::vector<SinglePbData> SensitivityStudy::get_output_data() const {
  return output_data;
}

void SensitivityStudy::run_capex_analysis() {
  logger->display_message("Capex analysis in progress");
  Analysis capex_analysis(input_data, "", logger, SensitivityPbType::CAPEX);
  std::pair<SinglePbData, SinglePbData> capex_data = capex_analysis.run();

  output_data.push_back(capex_data.first);
  output_data.push_back(capex_data.second);
}

void SensitivityStudy::run_projection_analysis() {
  logger->display_message("Projection analysis in progress");
  std::mutex m;
  std::for_each(
      std::execution::par_unseq, input_data.projection.begin(),
      input_data.projection.end(), [&](const std::string& candidate_name) {
        if (input_data.name_to_id.find(candidate_name) !=
            input_data.name_to_id.end()) {
          Analysis projection_analysis(input_data, candidate_name, logger,
                                       SensitivityPbType::PROJECTION);
          auto [minimizeSolution, maximizeSolution] = projection_analysis.run();
          std::lock_guard guard(m);
          output_data.push_back(minimizeSolution);
          output_data.push_back(maximizeSolution);
        } else {
          logger->display_message(
              "Warning: " + candidate_name +
              " ignored as it has not been found in the list "
              "of investment candidates");
        }
      });
}