#pragma once

#include "SimplexBasis.h"
#include "SlaveCut.h"
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
  SubproblemWorker() = default;
  SubproblemWorker(VariableMap const &variable_map, std::string const &path_to_mps,
              double const &slave_weight, const std::string &solver_name,
              const int log_level, const std::string &log_name);
  virtual ~SubproblemWorker() = default;

 public:
  void fix_to(Point const &x0) const;

  void get_subgradient(Point &s) const;

  SimplexBasis get_basis() const;
};
