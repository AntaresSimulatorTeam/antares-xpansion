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

    void get_subgradient(Point& s) const;
};
