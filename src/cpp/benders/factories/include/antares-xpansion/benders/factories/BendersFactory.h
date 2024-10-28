#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

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
  std::shared_ptr<Benders::Criterion::CriterionComputation>
      criterion_computation_ = nullptr;
  Logger logger_ = nullptr;
  Writer writer_ = nullptr;
  BENDERSMETHOD method_ = BENDERSMETHOD::BENDERS;
  std::string context_ = bendersmethod_to_string(BENDERSMETHOD::BENDERS);
  std::string positive_unsupplied_file_;
  static constexpr const char* const LOLD_FILE = "LOLD.txt";

  [[nodiscard]] int RunExternalLoop();
  [[nodiscard]] int RunBenders();
  [[nodiscard]] std::shared_ptr<MathLoggerDriver> BuildMathLogger(
      bool benders_log_console) const;
  pBendersBase PrepareForExecution(bool external_loop);
  [[nodiscard]] Benders::Criterion::OuterLoopInputData ProcessCriterionInput();

  Benders::Criterion::OuterLoopInputData BuildPatternsUsingAreaFile();
  std::set<std::string> ReadAreaFile();
  void EndMessage(const double execution_time);
  void AddCriterionOutput(std::shared_ptr<MathLoggerDriver> math_log_driver);

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