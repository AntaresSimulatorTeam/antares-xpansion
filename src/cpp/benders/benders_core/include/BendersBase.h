#pragma once

#include <execution>
#include <filesystem>

#include "BendersMathLogger.h"
#include "BendersStructsDatas.h"
#include "ILogger.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "SubproblemCut.h"
#include "SubproblemWorker.h"
#include "Timer.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "common.h"

/**
 * std execution policies don't share a base type so we can't just select them
 *in place in the foreach This function allow the selection of policy via
 *template deduction
 **/
template <class lambda>
auto selectPolicy(lambda f, bool shouldParallelize) {
  if (shouldParallelize)
    return f(std::execution::par_unseq);
  else
    return f(std::execution::seq);
}
class BendersBase {
 public:
  virtual ~BendersBase() = default;
  BendersBase(BendersBaseOptions options, Logger logger, Writer writer,
              std::shared_ptr<MathLoggerDriver> mathLoggerDriver);
  virtual void launch() = 0;
  void set_solver_log_file(const std::filesystem::path &log_file);
  [[nodiscard]] std::filesystem::path solver_log_file() const {
    return solver_log_file_;
  }
  double execution_time() const;
  virtual std::string BendersName() const = 0;
  void set_input_map(const CouplingMap &coupling_map);

 protected:
  CurrentIterationData _data;
  VariableMap master_variable_map_;
  CouplingMap coupling_map_;
  std::shared_ptr<MathLoggerDriver> mathLoggerDriver_;

 protected:
  virtual void free() = 0;
  virtual void Run() = 0;
  virtual void InitializeProblems() = 0;
  virtual void init_data();
  void update_best_ub();
  bool ShouldBendersStop();
  bool is_initial_relaxation_requested() const;
  bool SwitchToIntegerMaster(bool is_relaxed) const;
  virtual void UpdateTrace();
  virtual void ComputeXCut();
  void ComputeInvestCost();
  virtual void compute_ub();
  virtual void get_master_value();
  void GetSubproblemCut(SubProblemDataMap &subproblem_data_map);
  virtual void post_run_actions() const;
  void BuildCutFull(const SubProblemDataMap &subproblem_data_map);
  virtual void DeactivateIntegrityConstraints() const;
  virtual void ActivateIntegrityConstraints() const;
  virtual void SetDataPreRelaxation();
  virtual void ResetDataPostRelaxation();
  [[nodiscard]] std::filesystem::path GetSubproblemPath(
      std::string const &subproblem_name) const;
  [[nodiscard]] double SubproblemWeight(int subproblem_count,
                                        std::string const &name) const;
  [[nodiscard]] std::filesystem::path get_master_path() const;
  [[nodiscard]] std::filesystem::path get_structure_path() const;
  [[nodiscard]] LogData bendersDataToLogData(
      const CurrentIterationData &data) const;
  virtual void reset_master(WorkerMaster *worker_master);
  void free_master() const;
  void free_subproblems();
  void AddSubproblem(const std::pair<std::string, VariableMap> &kvp);
  [[nodiscard]] WorkerMasterPtr get_master() const;
  void MatchProblemToId();
  void AddSubproblemName(const std::string &name);
  [[nodiscard]] std::string get_master_name() const;
  [[nodiscard]] std::string get_solver_name() const;
  [[nodiscard]] int get_log_level() const;
  [[nodiscard]] bool is_trace() const;
  [[nodiscard]] Point get_x_cut() const;
  void set_x_cut(const Point &x0);
  [[nodiscard]] Point get_x_out() const;
  void set_x_out(const Point &x0);
  [[nodiscard]] double get_timer_master() const;
  void set_timer_master(const double &timer_master);
  [[nodiscard]] double GetSubproblemsWalltime() const;
  void SetSubproblemsWalltime(const double &duration);
  [[nodiscard]] double GetSubproblemsCpuTime() const;
  void SetSubproblemsCpuTime(const double &duration);
  [[nodiscard]] double GetSubproblemsCumulativeCpuTime() const;
  void SetSubproblemsCumulativeCpuTime(const double &duration);
  [[nodiscard]] double GetSubproblemCost() const;
  void SetSubproblemCost(const double &subproblem_cost);
  bool IsResumeMode() const;
  std::filesystem::path LastIterationFile() const {
    return std::filesystem::path(_options.LAST_ITERATION_JSON_FILE);
  }
  void UpdateMaxNumberIterationResumeMode(const unsigned nb_iteration_done);
  LogData GetBestIterationData() const;
  void SaveCurrentIterationInOutputFile() const;
  void SaveSolutionInOutputFile() const;
  void PrintCurrentIterationCsv();
  void OpenCsvFile();
  void CloseCsvFile();
  void ChecksResumeMode();
  virtual void SaveCurrentBendersData();
  void ClearCurrentIterationCutTrace() const;
  virtual void EndWritingInOutputFile() const;
  [[nodiscard]] int GetNumIterationsBeforeRestart() const {
    return iterations_before_resume;
  }
  double GetBendersTime() const;
  virtual void write_basis() const;
  // SubproblemsMapPtr GetSubProblemsMapPtr() { return subproblem_map; }
  SubproblemsMapPtr GetSubProblemMap() const { return subproblem_map; }
  StrVector GetSubProblemNames() const { return subproblems; }
  double AbsoluteGap() const { return _options.ABSOLUTE_GAP; }
  double RelativeGap() const { return _options.RELATIVE_GAP; }
  double RelaxedGap() const { return _options.RELAXED_GAP; }
  DblVector GetAlpha_i() const { return _data.single_subpb_costs_under_approx; }
  void SetAlpha_i(const DblVector &single_subpb_costs_under_approx) {
    _data.single_subpb_costs_under_approx = single_subpb_costs_under_approx;
  }
  int ProblemToId(const std::string &problem_name) const {
    return _problem_to_id.at(problem_name);
  }
  BendersBaseOptions Options() const { return _options; }
  virtual void UpdateStoppingCriterion();
  virtual bool ShouldRelaxationStop() const;
  int GetNumOfSubProblemsSolvedBeforeResume() {
    return cumulative_number_of_subproblem_resolved_before_resume;
  }

  void BoundSimplexIterations(int subproblem_iteration);
  void ResetSimplexIterationsBounds();

  SolverLogManager solver_log_manager_;

 private:
  void print_master_and_cut(std::ostream &file, int ite,
                            WorkerMasterDataPtr &trace, Point const &xopt);
  void print_master_csv(std::ostream &stream, const WorkerMasterDataPtr &trace,
                        Point const &xopt) const;
  void check_status(const SubProblemDataMap &subproblem_data_map) const;
  [[nodiscard]] LogData build_log_data_from_data() const;
  [[nodiscard]] Output::SolutionData solution() const;
  [[nodiscard]] std::string status_from_criterion() const;
  void compute_cut_aggregate(const SubProblemDataMap &subproblem_data_map);
  void compute_cut(const SubProblemDataMap &subproblem_data_map);
  [[nodiscard]] std::map<std::string, int> get_master_variable_map(
      const std::map<std::string, std::map<std::string, int>> &input_map) const;
  [[nodiscard]] virtual bool shouldParallelize() const = 0;
  Output::Iteration iteration(const WorkerMasterDataPtr &masterDataPtr_l) const;
  LogData FinalLogData() const;

 private:
  BendersBaseOptions _options;
  unsigned int _totalNbProblems = 0;
  std::filesystem::path solver_log_file_ = "";
  BendersRelevantIterationsData relevantIterationData_ = {
      std::make_shared<WorkerMasterData>(), nullptr};
  WorkerMasterPtr _master;
  VariableMap _problem_to_id;
  SubproblemsMapPtr subproblem_map;
  StrVector subproblems;
  std::ofstream _csv_file;
  std::filesystem::path _csv_file_path;
  LogData best_iteration_data;
  int iterations_before_resume = 0;
  int cumulative_number_of_subproblem_resolved_before_resume = 0;
  Timer benders_timer;

 public:
  Logger _logger;
  Writer _writer;
};
using pBendersBase = std::shared_ptr<BendersBase>;
