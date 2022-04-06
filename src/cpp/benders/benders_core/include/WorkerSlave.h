#pragma once

#include "SimplexBasis.h"
#include "SlaveCut.h"
#include "Worker.h"

/*!
 * \class WorkerSlave
 * \brief Class daughter of Worker Class, build and manage a slave problem
 */
class WorkerSlave;
typedef std::shared_ptr<WorkerSlave> WorkerSlavePtr;
typedef std::vector<WorkerSlavePtr> WorkerSlaves;
typedef std::map<std::string, WorkerSlavePtr> SlavesMapPtr;

class WorkerSlave : public Worker {
 public:
  WorkerSlave() = default;
  WorkerSlave(VariableMap const &variable_map,
              const std::filesystem::path &path_to_mps,
              double const &slave_weight, const std::string &solver_name,
              const int log_level, const std::filesystem::path &log_name);
  virtual ~WorkerSlave() = default;

 public:
  void fix_to(Point const &x0) const;

  void get_subgradient(Point &s) const;

  SimplexBasis get_basis() const;
};
