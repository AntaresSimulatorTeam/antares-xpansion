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

  WorkerMasterPtr _master;
  SlavesMapPtr _map_slaves;

  Str2Int _problem_to_id;
  BendersData _data;
  BendersBaseOptions _options;
  StrVector _slaves;

  AllCutStorage _all_cuts_storage;
  BendersTrace _trace;

  Logger _logger;
  Writer _writer;

  virtual void launch() = 0;

 protected:
  virtual void free() = 0;
  virtual void run() = 0;

  // map linking each problem name to its variables and their ids
  CouplingMap _input;
  int _nbWeeks = 0;

 public:
  void init_data();

  void print_csv();
  void print_csv_iteration(std::ostream &file, int ite);
  void print_master_and_cut(std::ostream &file, int ite,
                            WorkerMasterDataPtr &trace, Point const &xopt);
  void print_master_csv(std::ostream &stream, const WorkerMasterDataPtr &trace,
                        Point const &xopt) const;
  void print_cut_csv(std::ostream &stream, SlaveCutDataHandler const &handler,
                     std::string const &name, int const islaves) const;

  void update_best_ub();
  void bound_simplex_iter(int simplexiter);
  bool stopping_criterion();
  void update_trace();
  void check_status(AllCutPackage const &all_package) const;

  void get_master_value();
  void get_slave_cut(SlaveCutPackage &slave_cut_package);

  LogData build_log_data_from_data() const;
  void post_run_actions() const;
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

  void build_cut_full(const AllCutPackage &all_package);

  std::string get_slave_path(std::string const &slave_name) const;
  double slave_weight(int nslaves, std::string const &name) const;
  std::string get_master_path() const;
  std::string get_structure_path() const;
  LogData bendersDataToLogData(const BendersData &data);
};
using pBendersBase = std::shared_ptr<BendersBase>;