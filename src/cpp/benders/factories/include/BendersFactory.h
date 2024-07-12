#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "BendersMPI.h"
#include "common.h"

class BendersMainFactory {
 private:
  char** argv_;
  std::filesystem::path options_file_;
  boost::mpi::environment* penv_ = nullptr;
  boost::mpi::communicator* pworld_ = nullptr;
  SOLVER solver_ = SOLVER::BENDERS;
  [[nodiscard]] int RunExternalLoop() const;
  [[nodiscard]] int RunBenders() const;
  pBendersBase PrepareForExecution(BendersLoggerBase& benders_loggers,
                                   const SimulationOptions& options,
                                   bool adequacy_criterion) const;

 public:
  explicit BendersMainFactory(int argc, char** argv,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world,
                              const SOLVER& solver);
  explicit BendersMainFactory(int argc, char** argv,
                              const std::filesystem::path& options_file,
                              boost::mpi::environment& env,
                              boost::mpi::communicator& world,
                              const SOLVER& solver);
  int Run() const;
};
#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H