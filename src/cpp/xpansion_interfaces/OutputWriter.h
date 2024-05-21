
#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Output {

// string constantes
const std::string ANTARES_C("antares"), VERSION_C("version"),
    ANTARES_XPANSION_C("antares_xpansion"), BEGIN_C("begin"), END_C("end"),
    RUN_DURATION_C("run_duration"), MASTER_DURATION_C("master_duration"),
    SUBPROBLEM_DURATION_C("subproblem_duration"), ITERATIONS_C("iterations"),
    BEST_UB_C("best_ub"), CANDIDATES_C("candidates"), INVEST_C("invest"),
    MAX_C("max"), MIN_C("min"), NAME_C("name"),
    INVESTMENT_COST_C("investment_cost"), LB_C("lb"),
    OPERATIONAL_COST_C("operational_cost"), OPTIMALITY_GAP_C("optimality_gap"),
    OVERALL_COST_C("overall_cost"), RELATIVE_GAP_C("relative_gap"), UB_C("ub"),
    NBWEEKS_C("nbWeeks"), OPTIONS_C("options"), SOLUTION_C("solution"),
    OUTER_LOOP_SOLUTION_C("security criterion solution"),
    ITERATION_C("iteration"), PROBLEM_STATUS_C("problem_status"),
    OPTIMAL_C("OPTIMAL"), LIMIT_REACHED_C("limit reached"), ERROR_C("ERROR"),
    VALUES_C("values"), STOPPING_CRITERION_C("stopping_criterion"),
    MASTER_NAME_C("MASTER_NAME"), LOG_LEVEL_C("LOG_LEVEL"),
    SOLVER_NAME_C("SOLVER_NAME"), PROBLEMNAME_C("problem_name"),
    PROBLEMPATH_C("problem_path"),
    CUMULATIVE_NUMBER_OF_SUBPROBLEM_RESOLVED_C(
        "cumulative_number_of_subproblem_resolutions");
struct CandidateData {
  std::string name;
  double invest;
  double min;
  double max;
};
typedef std::vector<CandidateData> CandidatesVec;
struct Iteration {
  double master_duration;
  double subproblem_duration;
  double lb;
  double ub;
  double best_ub;
  double optimality_gap;
  double relative_gap;
  double investment_cost;
  double operational_cost;
  double overall_cost;
  CandidatesVec candidates;
  int cumulative_number_of_subproblem_resolved;
};
typedef std::vector<Iteration> Iterations;
/*!
 *  \brief struct saves some entries to be later written to the json file
 *
 *   nbWeeks_p : number of the weeks in the study
 *   solution : solution data as iteration
 */
struct SolutionData {
  Iteration solution;
  int nbWeeks_p;
  int best_it;
  std::string problem_status;
  std::string stopping_criterion;
};
/*!
 *  \brief struct containing some entries to be later written to the json file
 *
 *  nbWeeks_p : number of the weeks in the study
 *   bendersTrace_p : trace to be written ie iterations details
 *   bendersData_p : final benders data to get the best iteration
 *   min_abs_gap : minimum absolute gap wanted
 *   min_rel_gap : minimum relative gap wanted
 *   max_iter : maximum number of iterations
 */
struct IterationsData {
  int nbWeeks_p;
  double elapsed_time;
  Iterations iters;
  SolutionData solution_data;
};
struct ProblemData {
  std::string name;
  std::filesystem::path path;
  std::string status;
};
/*!
 * \class OutputWriter
 * \brief OutputWriter class to describe the execuion session of an antares
 * xpansion optimization in a log file
 */
class OutputWriter {
 public:
  /*!
   *  \brief destructor of class OutputWriter
   */
  virtual ~OutputWriter() = default;

  /*!
   *  \brief  saves some entries to be later written to the json file
   *
   *  \param solution_data containing solution data
   */
  virtual void update_solution(const SolutionData &solution_data) = 0;

  /*!
   *  \brief write the log data into a file
   */
  virtual void dump() = 0;

  /*!
   * \brief initialize outputs
   */
  virtual void initialize() = 0;

  virtual void end_writing(const IterationsData &iterations_data) = 0;

  virtual void write_solver_name(const std::string &solver_name) = 0;
  virtual void write_master_name(const std::string &master_name) = 0;
  virtual void write_log_level(const int log_level) = 0;
  virtual void write_solution(const SolutionData &solution) = 0;
  virtual void write_iteration(const Iteration &iteration_data,
                               const size_t iteration_num) = 0;
  virtual void updateBeginTime() = 0;
  virtual void updateEndTime() = 0;
  virtual void write_nbweeks(const int nb_weeks) = 0;
  virtual void write_duration(const double duration) = 0;
  virtual std::string solution_status() const = 0;
  virtual void WriteProblem(const ProblemData &problem_data) = 0;
};
}  // namespace Output
using Writer = std::shared_ptr<Output::OutputWriter>;