#pragma once

#include "SubproblemCut.h"
#include "Worker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "common.h"

struct CriteriaCurrentIterationData
{
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

// Solution points flowing between master and cuts stages each iteration
struct SolutionPointData
{
    Point x_in;
    Point x_out;
    Point x_cut;
    Point min_invest;
    Point max_invest;
    std::vector<double> master_only_vars_out;
    std::vector<double> master_only_vars_in;
    std::vector<double> master_only_vars_cut;
};

// Master problem solver outputs
struct MasterData
{
    double lb = -1e20;
    int master_status = SOLVER_STATUS::UNKNOWN;
    double timer_master = 0;
    double invest_cost = 0;
    std::vector<double> single_subpb_costs_under_approx;
    double overall_subpb_cost_under_approx = 0;
};

// Subproblem solving and cuts stage outputs
struct SubproblemCutsData
{
    double ub = +1e20;
    double subproblem_cost = 0;
    int min_simplexiter = 0;
    int max_simplexiter = 0;
    double subproblems_walltime = 0;
    double subproblems_cputime = 0;
    double subproblems_cumulative_cputime = 0;
};

// Main loop orchestration and convergence tracking
struct IterationControlData
{
    int it = 0;
    bool stop = false;
    StoppingCriterion stopping_criterion = StoppingCriterion::empty;
    bool is_in_initial_relaxation = false;
    double iteration_time = 0;
    double benders_time = 0;
    double best_ub = +1e20;
    int best_it = 0;
    int nsubproblem = 0;
    int number_of_subproblem_solved = 0;
    int cumulative_number_of_subproblem_solved = 0;
};

/*! \struct
 * struct that hold current Benders iteration data, composed of focused sub-structures
 */
struct CurrentIterationData
{
    SolutionPointData solution;
    MasterData master;
    SubproblemCutsData cuts;
    IterationControlData control;
    CriteriaCurrentIterationData criteria;
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
class WorkerMasterData
{
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

struct BendersRelevantIterationsData
{
    WorkerMasterData last;
    WorkerMasterData best;
};

using WorkerMasterDataVect = std::vector<WorkerMasterData>;
