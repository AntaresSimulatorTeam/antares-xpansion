#pragma once

#include "ILogger.h"
#include "SubproblemCut.h"
#include "Worker.h"
#include "common.h"

/*! \struct struct that hold current benders iteration
 */
struct CurrentIterationData {
  double subproblems_walltime;
  double subproblems_cputime;
  double subproblems_cumulative_cputime;
  double timer_master;
  double lb;
  double ub;
  double best_ub;
  int deletedcut;
  int it;
  double iteration_time;
  bool stop;
  double overall_subpb_cost_under_approx;
  std::vector<double> single_subpb_costs_under_approx;
  double subproblem_cost;
  double invest_cost;
  int best_it;
  Point x_in;
  Point x_out;
  Point x_cut;
  Point min_invest;
  Point max_invest;
  int nsubproblem;
  int master_status;
  double benders_time;
  StoppingCriterion stopping_criterion;
  bool is_in_initial_relaxation;
  int number_of_subproblem_solved;
  int cumulative_number_of_subproblem_solved;
  int min_simplexiter;
  int max_simplexiter;
  // ugly
  int benders_num_run;
  double outer_loop_criterion;
};

// /*! \struct to store benders cuts data
//  */
// struct BendersCuts {
//   Point x_cut;
//   SubProblemDataMap subsProblemDataMap;
// };

// using BendersCutsPerIteration = std::vector<BendersCuts>;

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

struct BendersRelevantIterationsData {
  WorkerMasterData last;
  WorkerMasterData best;
};
using WorkerMasterDataVect = std::vector<WorkerMasterData>;
