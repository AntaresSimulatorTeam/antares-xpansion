#include "JsonWriter.h"

#include "config.h"

namespace clock_utils {
std::string timeToStr(const std::time_t &time_p) {
  struct tm local_time;
  localtime_platform(time_p, local_time);
  // localtime_r(&time_p, &local_time); // Compliant
  const char *FORMAT = "%d-%m-%Y %H:%M:%S";
  char buffer_l[100];
  strftime(buffer_l, sizeof(buffer_l), FORMAT, &local_time);
  std::string strTime_l(buffer_l);

  return strTime_l;
}
}  // namespace clock_utils
namespace Output {
JsonWriter::JsonWriter(std::shared_ptr<Clock> p_clock,
                       const std::string &json_filename)
    : _clock(p_clock), _filename(json_filename) {}

void JsonWriter::initialize(const BendersOptions &options) {
  write_options(options);
  updateBeginTime();
  dump();
}

void JsonWriter::updateBeginTime() {
  _output[BEGIN_C] = clock_utils::timeToStr(_clock->getTime());
}

void JsonWriter::updateEndTime() {
  _output[END_C] = clock_utils::timeToStr(_clock->getTime());
}

void JsonWriter::write_options(BendersOptions const &bendersOptions_p) {
// Options
#define BENDERS_OPTIONS_MACRO(name__, type__, default__) \
  _output[OPTIONS_C][#name__] = bendersOptions_p.name__;
#include "BendersOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
}

void JsonWriter::write_iterations(const IterationsData &iterations_data) {
  _output[NBWEEKS_C] = iterations_data.nbWeeks_p;
  _output[DURATION_C] = iterations_data.elapsed_time;

  // Iterations
  size_t iterCnt_l(0);
  for (const auto &iter : iterations_data.iters) {
    ++iterCnt_l;

    std::string strIterCnt_l(std::to_string(iterCnt_l));
    _output[ITERATIONS_C][strIterCnt_l][DURATION_C] = iter.time;
    _output[ITERATIONS_C][strIterCnt_l][LB_C] = iter.lb;
    _output[ITERATIONS_C][strIterCnt_l][UB_C] = iter.ub;
    _output[ITERATIONS_C][strIterCnt_l][BEST_UB_C] = iter.best_ub;
    _output[ITERATIONS_C][strIterCnt_l][OPTIMALITY_GAP_C] = iter.optimality_gap;
    _output[ITERATIONS_C][strIterCnt_l][RELATIVE_GAP_C] = iter.relative_gap;
    _output[ITERATIONS_C][strIterCnt_l][INVESTMENT_COST_C] =
        iter.investment_cost;
    _output[ITERATIONS_C][strIterCnt_l][OPERATIONAL_COST_C] =
        iter.operational_cost;
    _output[ITERATIONS_C][strIterCnt_l][OVERALL_COST_C] = iter.overall_cost;

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

  // solution
  _output[SOLUTION_C][ITERATION_C] = iterations_data.solution_data.best_it;
  _output[SOLUTION_C][INVESTMENT_COST_C] =
      iterations_data.solution_data.solution.investment_cost;
  _output[SOLUTION_C][OPERATIONAL_COST_C] =
      iterations_data.solution_data.solution.operational_cost;
  _output[SOLUTION_C][OVERALL_COST_C] =
      iterations_data.solution_data.solution.overall_cost;

  for (const auto &candidate :
       iterations_data.solution_data.solution.candidates) {
    _output[SOLUTION_C][VALUES_C][candidate.name] = candidate.invest;
  }

  _output[SOLUTION_C][OPTIMALITY_GAP_C] =
      iterations_data.solution_data.solution.optimality_gap;
  _output[SOLUTION_C][RELATIVE_GAP_C] =
      iterations_data.solution_data.solution.relative_gap;

  _output[SOLUTION_C][STOPPING_CRITERION_C] =
      iterations_data.solution_data.stopping_criterion;
  _output[SOLUTION_C][PROBLEM_STATUS_C] =
      iterations_data.solution_data.problem_status;
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
  _output[ANTARES_C][VERSION_C] = ANTARES_VERSION_TAG;
  _output[ANTARES_XPANSION_C][VERSION_C] = PROJECT_VER;

  std::ofstream jsonOut_l(_filename);
  if (jsonOut_l) {
    // Output
    jsonOut_l << _output << std::endl;
  } else {
    std::cout << "Impossible d'ouvrir le fichier json " << _filename
              << std::endl;
  }
}

void JsonWriter::end_writing(const IterationsData &iterations_data) {
  updateEndTime();
  write_iterations(iterations_data);
  dump();
}
}  // namespace Output