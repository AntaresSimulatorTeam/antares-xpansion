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

class SubproblemWorker : public Worker {
 public:
  explicit SubproblemWorker(Logger logger) : Worker(logger) {}
  SubproblemWorker(VariableMap const &variable_map,
                   const std::filesystem::path &path_to_mps,
                   double const &slave_weight, const std::string &solver_name,
                   const int log_level,
                   SolverLogManager&solver_log_manager,
                   Logger logger);
  virtual ~SubproblemWorker() = default;
  void get_solution(std::vector<double> &solution) const;

 public:
  void fix_to(Point const &x0) const;

  void get_subgradient(Point &s) const;
};
