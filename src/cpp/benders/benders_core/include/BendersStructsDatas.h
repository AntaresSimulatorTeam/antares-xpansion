#pragma once

#include "SubproblemCut.h"
#include "Worker.h"
#include "common.h"
#include "core/ILogger.h"

struct BendersData {
  int nbasis;
  double subproblem_timers;
  double timer_master;
  double lb;
  double ub;
  double best_ub;
  int maxsimplexiter;
  int minsimplexiter;
  int deletedcut;
  int it;
  bool stop;
  double alpha;
  std::vector<double> alpha_i;
  double subproblem_cost;
  double invest_cost;
  int best_it;
  Point bestx;
  Point x0;
  Point min_invest;
  Point max_invest;
  int nsubproblem;
  double dnslaves;
  int master_status;
  double elapsed_time;
  StoppingCriterion stopping_criterion;
  bool is_in_initial_relaxation;
};
/*!
 * \class WorkerMasterData
 * \brief Class use to store trace information during the algorithm run
 */
class WorkerMasterData {
 public:
  WorkerMasterData() = default;

  bool _valid = false;
  double _lb;
  double _ub;
  double _best_ub;
  int _deleted_cut;
  int _nbasis;
  double _time;
  PointPtr _x0;
  PointPtr _min_invest;
  PointPtr _max_invest;
  std::map<std::string, SubproblemCutDataPtr> _cut_trace;

  double _invest_cost;
  double _operational_cost;

  Point get_point() const;
  Point get_min_invest() const;
  Point get_max_invest() const;
};

using WorkerMasterDataPtr = std::shared_ptr<WorkerMasterData>;
using BendersTrace = std::vector<WorkerMasterDataPtr>;

LogData defineLogDataFromBendersDataAndTrace(const BendersData& data,
                                             const BendersTrace& trace);
