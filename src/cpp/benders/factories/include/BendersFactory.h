#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "BendersMPI.h"
#include "CriterionComputation.h"
#include "common.h"

struct InvalidStructureFile
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class BendersMainFactory {
 private:
  char** argv_;
  boost::mpi::environment* penv_ = nullptr;
  boost::mpi::communicator* pworld_ = nullptr;
  SOLVER solver_ = SOLVER::BENDERS;
  SimulationOptions options_;
  BendersLoggerBase benders_loggers_;
  std::shared_ptr<Outerloop::CriterionComputation> criterion_computation_ =
      nullptr;
  Logger logger_ = nullptr;
  Writer writer_ = nullptr;

  [[nodiscard]] int RunExternalLoop();
  [[nodiscard]] int RunBenders();
  [[nodiscard]] std::shared_ptr<MathLoggerDriver> BuildMathLogger(
      const BENDERSMETHOD& method, bool benders_log_console) const;
  pBendersBase PrepareForExecution(bool external_loop);
  [[nodiscard]] Outerloop::OuterLoopInputData ProcessCriterionInput(
      const CouplingMap& couplingMap);

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
  int Run();
  std::filesystem::path LogReportsName() const;
};
#endif  // ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H