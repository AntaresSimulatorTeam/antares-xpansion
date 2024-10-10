#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "BendersMPI.h"
#include "CriterionComputation.h"
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
  std::shared_ptr<MathLoggerDriver> BuildMathLogger(
      const SimulationOptions& options,
      const std::shared_ptr<Outerloop::CriterionComputation>
          criterion_computation,
      const std::filesystem::path& math_logs_file, const BENDERSMETHOD& method,
      bool benders_log_console) const;
  pBendersBase PrepareForExecution(
      BendersLoggerBase& benders_loggers, const SimulationOptions& options,
      bool external_loop,
      std::shared_ptr<Outerloop::CriterionComputation> computation =
          nullptr) const;

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