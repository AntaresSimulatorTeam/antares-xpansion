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
                     const std::filesystem::path& path_to_mps,
                     double slave_weight,
                     const std::string& solver_name,
                     int log_level,
                     const SolverLogManager& solver_log_manager,
                     Logger logger,
                     ProblemsFormat format);
    SubproblemWorker(const std::filesystem::path& path_to_mps,
                     const double& slave_weight,
                     const std::string& solver_name,
                     const int log_level,
                     SolverLogManager& solver_log_manager,
                     Logger logger,
                     ProblemsFormat format);
    virtual ~SubproblemWorker() = default;
    std::vector<double> get_solution() const;

public:
    void fix_to(const Point& x0) const;
    void fix_rhs_to(const std::string& name, const double& value) const;
    double get_rhs_value_from_name(const std::string& constraint_name) const;

    void get_subgradient(Point& s) const;
};
