#pragma once

#include "BendersStructsDatas.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "SlaveCut.h"
#include "Worker.h"
#include "WorkerMaster.h"
#include "WorkerSlave.h"
#include "common.h"
#include "core/ILogger.h"

class BendersBase {
 public:
  virtual ~BendersBase() = default;
  BendersBase(BendersBaseOptions const &options, Logger &logger, Writer writer);
  virtual void launch() = 0;
  void set_log_file(const std::string &log_name);
  std::string log_name() const { return _log_name; }

 protected:
  BendersData _data;
  // map linking each problem name to its variables and their ids
  CouplingMap _input;
  Str2Int master_variable_map;
  CouplingMap slaves_map;

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
  void get_slave_cut(SlaveCutPackage &slave_cut_package);
  void post_run_actions() const;
  void build_cut_full(const AllCutPackage &all_package);
  std::string get_slave_path(std::string const &slave_name) const;
  double slave_weight(int nslaves, std::string const &name) const;
  std::string get_master_path() const;
  std::string get_structure_path() const;
  LogData bendersDataToLogData(const BendersData &data) const;
  void build_input_map();
  CouplingMap get_input() const;
  void push_in_trace(const WorkerMasterDataPtr &worker_master_data);
  void reset_master(WorkerMaster *worker_master);
  void free_master() const;
  void free_slaves();
  void add_slave(const std::pair<std::string, Str2Int> &kvp);
  WorkerMasterPtr get_master() const;
  int get_totalNbProblems() const;
  void set_problem_to_id(const std::string &name, const int id);
  void set_cut_storage();
  void add_slave_name(const std::string &name);
  int get_slaves_number() const;
  std::string get_master_name() const;
  std::string get_solver_name() const;
  int get_log_level() const;
  bool is_trace() const;
  Point get_x0() const;
  void set_x0(const Point &x0);
  double get_timer_master() const;
  void set_timer_master(const double &timer_master);
  double get_timer_slaves() const;
  void set_timer_slaves(const double &timer_slaves);
  double get_slave_cost() const;
  void set_slave_cost(const double &slave_cost);

 private:
  void print_csv_iteration(std::ostream &file, int ite);
  void print_master_and_cut(std::ostream &file, int ite,
                            WorkerMasterDataPtr &trace, Point const &xopt);
  void print_master_csv(std::ostream &stream, const WorkerMasterDataPtr &trace,
                        Point const &xopt) const;
  void print_cut_csv(std::ostream &stream, SlaveCutDataHandler const &handler,
                     std::string const &name, int const islaves) const;
  void bound_simplex_iter(int simplexiter);
  void check_status(AllCutPackage const &all_package) const;
  LogData build_log_data_from_data() const;
  Output::IterationsData output_data() const;
  Output::Iteration iteration(const WorkerMasterDataPtr &masterDataPtr_l) const;
  Output::CandidatesVec candidates_data(
      const WorkerMasterDataPtr &masterDataPtr_l) const;
  Output::SolutionData solution() const;
  std::string status_from_criterion() const;
  void compute_cut_val(const SlaveCutDataHandlerPtr &handler, const Point &x0,
                       Point &s) const;
  void compute_cut_aggregate(const AllCutPackage &all_package);
  void compute_cut(const AllCutPackage &all_package);
  std::map<std::string, int> get_master_variable_map(
      std::map<std::string, std::map<std::string, int>> input_map) const;
  CouplingMap get_slaves_map(CouplingMap input) const;

 private:
  BendersBaseOptions _options;
  int _totalNbProblems = 0;
  std::string _log_name = "";
  BendersTrace _trace;
  WorkerMasterPtr _master;
  Str2Int _problem_to_id;
  SlavesMapPtr _map_slaves;
  AllCutStorage _all_cuts_storage;
  StrVector _slaves;

 public:
  Logger _logger;
  Writer _writer;
};
using pBendersBase = std::shared_ptr<BendersBase>;
