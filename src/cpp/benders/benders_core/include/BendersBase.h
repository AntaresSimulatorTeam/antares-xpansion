#pragma once

#include <filesystem>

#include "BendersStructsDatas.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "SubproblemCut.h"
#include "SubproblemWorker.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "common.h"
#include "core/ILogger.h"

class BendersBase {
 public:
  virtual ~BendersBase() = default;
  BendersBase(BendersBaseOptions options, Logger &logger, Writer writer);
  virtual void launch() = 0;
  void set_log_file(const std::filesystem::path &log_name);
  [[nodiscard]] std::filesystem::path log_name() const { return _log_name; }

 protected:
  BendersData _data;
  VariableMap master_variable_map;
  CouplingMap coupling_map;

 protected:
  virtual void free() = 0;
  virtual void run() = 0;
  virtual void initialize_problems() = 0;
  void init_data();
  void print_csv();
  void update_best_ub();
  bool stopping_criterion();
  void update_trace();
  void get_master_value();
  void getSubproblemCut(SubproblemCutPackage &subproblem_cut_package);
  void post_run_actions() const;
  void build_cut_full(const AllCutPackage &all_package);
  [[nodiscard]] std::filesystem::path GetSubproblemPath(
      std::string const &subproblem_name) const;
  [[nodiscard]] double SubproblemWeight(int subproblem_count,
                                        std::string const &name) const;
  [[nodiscard]] std::filesystem::path get_master_path() const;
  [[nodiscard]] std::filesystem::path get_structure_path() const;
  [[nodiscard]] static LogData bendersDataToLogData(const BendersData &data);
  void build_input_map();
  void push_in_trace(const WorkerMasterDataPtr &worker_master_data);
  void reset_master(WorkerMaster *worker_master);
  void free_master() const;
  void free_subproblems();
  void addSubproblem(const std::pair<std::string, VariableMap> &kvp);
  [[nodiscard]] WorkerMasterPtr get_master() const;
  void match_problem_to_id();
  void set_cut_storage();
  void AddSubproblemName(const std::string &name);
  [[nodiscard]] std::string get_master_name() const;
  [[nodiscard]] std::string get_solver_name() const;
  [[nodiscard]] int get_log_level() const;
  [[nodiscard]] bool is_trace() const;
  [[nodiscard]] Point get_x0() const;
  void set_x0(const Point &x0);
  [[nodiscard]] double get_timer_master() const;
  void set_timer_master(const double &timer_master);
  [[nodiscard]] double GetSubproblemTimers() const;
  void SetSubproblemTimers(const double &subproblem_timer);
  [[nodiscard]] double GetSubproblemCost() const;
  void SetSubproblemCost(const double &subproblem_cost);
  bool is_resume_mode();

 private:
  void print_csv_iteration(std::ostream &file, int ite);
  void print_master_and_cut(std::ostream &file, int ite,
                            WorkerMasterDataPtr &trace, Point const &xopt);
  void print_master_csv(std::ostream &stream, const WorkerMasterDataPtr &trace,
                        Point const &xopt) const;
  void bound_simplex_iter(int simplexiter);
  void check_status(AllCutPackage const &all_package) const;
  [[nodiscard]] LogData build_log_data_from_data() const;
  [[nodiscard]] Output::IterationsData output_data() const;
  [[nodiscard]] Output::SolutionData solution() const;
  [[nodiscard]] std::string status_from_criterion() const;
  void compute_cut_aggregate(const AllCutPackage &all_package);
  void compute_cut(const AllCutPackage &all_package);
  [[nodiscard]] std::map<std::string, int> get_master_variable_map(
      std::map<std::string, std::map<std::string, int>> input_map) const;
  [[nodiscard]] CouplingMap GetCouplingMap(CouplingMap input) const;
  [[nodiscard]] virtual bool shouldParallelize() const = 0;

 private:
  BendersBaseOptions _options;
  unsigned int _totalNbProblems = 0;
  std::filesystem::path _log_name;
  BendersTrace _trace;
  WorkerMasterPtr _master;
  VariableMap _problem_to_id;
  SubproblemsMapPtr subproblem_map;
  AllCutStorage _all_cuts_storage;
  StrVector subproblems;

 public:
  Logger _logger;
  Writer _writer;
};
using pBendersBase = std::shared_ptr<BendersBase>;
