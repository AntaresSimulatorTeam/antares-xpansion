#pragma once

#include "SubproblemCut.h"
#include "Worker.h"

/*!
 * \class SubproblemWorker
 * \brief Class daughter of Worker Class, build and manage a subproblem
 */
class SubproblemWorker;
typedef std::shared_ptr<SubproblemWorker> SubproblemWorkerPtr;
typedef std::vector<SubproblemWorkerPtr> WorkerSlaves;
typedef std::map<std::string, SubproblemWorkerPtr> SubproblemsMapPtr;

class SubproblemWorker: public Worker
{
public:
    using Worker::Worker;
    SubproblemWorker(const VariableMap& variable_map,
                     double slave_weight,
                     const std::string& solver_name,
                     int log_level,
                     const SolverLogManager& solver_log_manager,
                     Logger logger,
                     ProblemsFormat format,
                     IBendersProblemProvider* benders_problem_provider);
    SubproblemWorker(VariableMap& variable_map,
                     double slave_weight,
                     std::shared_ptr<SolverAbstract> solver,
                     Logger logger);
    void setup_obj(double slave_weight);
    virtual ~SubproblemWorker() = default;
    std::vector<double> get_solution() const;
    int get_variable_index(const std::string& variable_name);
    void delete_rows(int start_pos);
    int get_problem_row_num();

public:
    void fix_to(const Point& x0) const;

    void get_subgradient(Point& subgradient) const;
};
