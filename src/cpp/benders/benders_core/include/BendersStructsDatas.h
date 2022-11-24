#pragma once

#include "SubproblemCut.h"
#include "Worker.h"
#include "common.h"
#include "core/ILogger.h"

struct CurrentIterationData {
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
  Point x_in;
  Point x_out;
  Point x_cut;
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
  double _master_duration;
  double _subproblem_duration;
  PointPtr _x_in;
  PointPtr _x_out;
  PointPtr _x_cut;
  PointPtr _min_invest;
  PointPtr _max_invest;
  SubProblemDataMap _cut_trace;

  double _invest_cost;
  double _operational_cost;

  Point get_x_cut() const;
  Point get_min_invest() const;
  Point get_max_invest() const;
};

using WorkerMasterDataPtr = std::shared_ptr<WorkerMasterData>;
using BendersTrace = std::vector<WorkerMasterDataPtr>;
