#include "JsonWriter.h"

#include <iostream>

#include "config.h"

namespace Output {
JsonWriter::JsonWriter(std::shared_ptr<Clock> p_clock,
                       const std::filesystem::path &json_filename)
    : _clock(p_clock), _filename(json_filename) {}

JsonWriter::JsonWriter(const std::filesystem::path &json_filename,
                       const Json::Value &json_file_content)
    : _clock(std::make_shared<Clock>()),
      _filename(json_filename),
      _output(json_file_content) {}

void JsonWriter::_open_file() {
  auto parent_path = _filename.parent_path();
  if (!std::filesystem::exists(parent_path) &&
      !std::filesystem::create_directories(parent_path)) {
    std::cerr << "Could not create " << _filename.filename()
              << " parent folder: " << parent_path << std::endl;
  }
  _jsonOut_l.open(_filename, std::ofstream::out | std::ofstream::trunc);
  if (_jsonOut_l.fail()) {
    std::cout << "Could not open json file: " << _filename << std::endl;
  }
}

void JsonWriter::initialize() {
  updateBeginTime();
  dump();
}

void JsonWriter::updateBeginTime() {
  _output[BEGIN_C] = clock_utils::timeToStr(_clock->getTime());
}

void JsonWriter::updateEndTime() {
  _output[END_C] = clock_utils::timeToStr(_clock->getTime());
}

void JsonWriter::write_iterations(const IterationsData &iterations_data) {
  write_nbweeks(iterations_data.nbWeeks_p);
  write_duration(iterations_data.elapsed_time);

  // Iterations
  size_t iterCnt_l(0);
  for (const auto &iter : iterations_data.iters) {
    ++iterCnt_l;
    write_iteration(iter, iterCnt_l);
  }
  write_solution(iterations_data.solution_data);
}
void JsonWriter::write_nbweeks(const int nb_weeks) {
  _output[NBWEEKS_C] = nb_weeks;
}
void JsonWriter::write_duration(const double duration) {
  _output[RUN_DURATION_C] = duration;
}

void JsonWriter::write_iteration(const Iteration &iter,
                                 const size_t iteration_num) {
  std::string strIterCnt_l(std::to_string(iteration_num));

  _output[ITERATIONS_C][strIterCnt_l][MASTER_DURATION_C] = iter.master_duration;
  _output[ITERATIONS_C][strIterCnt_l][SUBPROBLEM_DURATION_C] =
      iter.subproblem_duration;
  _output[ITERATIONS_C][strIterCnt_l][LB_C] = iter.lb;
  _output[ITERATIONS_C][strIterCnt_l][UB_C] = iter.ub;
  _output[ITERATIONS_C][strIterCnt_l][BEST_UB_C] = iter.best_ub;
  _output[ITERATIONS_C][strIterCnt_l][OPTIMALITY_GAP_C] = iter.optimality_gap;
  _output[ITERATIONS_C][strIterCnt_l][RELATIVE_GAP_C] = iter.relative_gap;
  _output[ITERATIONS_C][strIterCnt_l][INVESTMENT_COST_C] = iter.investment_cost;
  _output[ITERATIONS_C][strIterCnt_l][OPERATIONAL_COST_C] =
      iter.operational_cost;
  _output[ITERATIONS_C][strIterCnt_l][OVERALL_COST_C] = iter.overall_cost;
  _output[ITERATIONS_C][strIterCnt_l]
         [CUMULATIVE_NUMBER_OF_SUBPROBLEM_RESOLVED_C] =
             iter.cumulative_number_of_subproblem_resolved;

  Json::Value vectCandidates_l(Json::arrayValue);
  for (const auto &candidate : iter.candidates) {
    Json::Value candidate_l;
    candidate_l[NAME_C] = candidate.name;
    candidate_l[INVEST_C] = candidate.invest;
    candidate_l[MIN_C] = candidate.min;
    candidate_l[MAX_C] = candidate.max;
    vectCandidates_l.append(candidate_l);
  }
  _output[ITERATIONS_C][strIterCnt_l][CANDIDATES_C] = vectCandidates_l;
}

void JsonWriter::write_solution(const SolutionData &solution) {
    // solution
  _output[SOLUTION_C][ITERATION_C] = solution.best_it;
  _output[SOLUTION_C][INVESTMENT_COST_C] = solution.solution.investment_cost;
  _output[SOLUTION_C][OPERATIONAL_COST_C] = solution.solution.operational_cost;
  _output[SOLUTION_C][OVERALL_COST_C] = solution.solution.overall_cost;

  for (const auto &candidate : solution.solution.candidates) {
    _output[SOLUTION_C][VALUES_C][candidate.name] = candidate.invest;
  }

  _output[SOLUTION_C][OPTIMALITY_GAP_C] = solution.solution.optimality_gap;
  _output[SOLUTION_C][RELATIVE_GAP_C] = solution.solution.relative_gap;

  _output[SOLUTION_C][STOPPING_CRITERION_C] = solution.stopping_criterion;
  _output[SOLUTION_C][PROBLEM_STATUS_C] = solution.problem_status;
}

void JsonWriter::update_solution(const SolutionData &solution_data) {
  _output[NBWEEKS_C] = solution_data.nbWeeks_p;

  _output[SOLUTION_C][INVESTMENT_COST_C] =
      solution_data.solution.investment_cost;
  _output[SOLUTION_C][OPERATIONAL_COST_C] =
      solution_data.solution.operational_cost;
  _output[SOLUTION_C][OVERALL_COST_C] = solution_data.solution.overall_cost;
  _output[SOLUTION_C][LB_C] = solution_data.solution.lb;
  _output[SOLUTION_C][UB_C] = solution_data.solution.ub;
  _output[SOLUTION_C][OPTIMALITY_GAP_C] = solution_data.solution.optimality_gap;
  _output[SOLUTION_C][RELATIVE_GAP_C] = solution_data.solution.relative_gap;

  _output[SOLUTION_C][PROBLEM_STATUS_C] = solution_data.problem_status;
  _output[SOLUTION_C][STOPPING_CRITERION_C] = solution_data.stopping_criterion;
  for (const auto &candidate : solution_data.solution.candidates) {
    _output[SOLUTION_C][VALUES_C][candidate.name] = candidate.invest;
  }

  updateEndTime();
}

/*!
 *  \brief write the json data into a file
 */
void JsonWriter::dump() {
  _open_file();
  _output[ANTARES_C][VERSION_C] = ANTARES_VERSION_TAG;
  _output[ANTARES_XPANSION_C][VERSION_C] = PROJECT_VER;

  // Output
  _jsonOut_l << _output << std::endl;
  _jsonOut_l.close();
}

void JsonWriter::end_writing(const IterationsData &iterations_data) {
  updateEndTime();
  write_iterations(iterations_data);
  dump();
}

void JsonWriter::write_solver_name(const std::string &solver_name) {
  _output[OPTIONS_C][SOLVER_NAME_C] = solver_name;
}
void JsonWriter::write_master_name(const std::string &master_name) {
  _output[OPTIONS_C][MASTER_NAME_C] = master_name;
}
void JsonWriter::write_log_level(const int log_level) {
  _output[OPTIONS_C][LOG_LEVEL_C] = log_level;
}

std::string JsonWriter::solution_status() const {
  return _output[SOLUTION_C][PROBLEM_STATUS_C].asString();
}

void JsonWriter::WriteProblem(const ProblemData &problem_data) {
  _output[ERROR_C][PROBLEMNAME_C] = problem_data.name;
  _output[ERROR_C][PROBLEMPATH_C] = problem_data.path.string();
  _output[ERROR_C][PROBLEM_STATUS_C] = problem_data.status;
}
}  // namespace Output