#pragma once

#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "SubproblemCut.h"
#include "Worker.h"
#include "common.h"

struct CriteriaCurrentIterationData {
  int benders_num_run = 0;
  std::vector<double> criteria = {};
  std::vector<double> patterns_values = {};
  double max_criterion = 0.;
  double max_criterion_best_it = 0.;
  double outer_loop_bilevel_best_ub = +1e20;
  double lambda = 0.;
  double lambda_min = 0.;
  double lambda_max = 0.;
  std::string max_criterion_area = "N/A";
  std::string max_criterion_area_best_it = "N/A";
};
/*! \struct
 * struct that hold current Benders iteration
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
  CriteriaCurrentIterationData criteria_current_iteration_data;
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
