#ifndef ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#define ANTARES_XPANSION_SRC_CPP_BENDERS_FACTORIES_INCLUDE_BENDERSFACTORY_H
#include <variant>

#include "antares-xpansion/benders/benders_core/CriterionComputation.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"

class BendersMainFactory {
 private:
  char** argv_;
  boost::mpi::environment* penv_ = nullptr;
  boost::mpi::communicator* pworld_ = nullptr;
  SOLVER solver_ = SOLVER::BENDERS;
  SimulationOptions options_;
  BendersLoggerBase benders_loggers_;
  std::variant<Benders::Criterion::CriterionInputData,
               Benders::Criterion::OuterLoopCriterionInputData>
      criterion_input_holder_;
  pBendersBase benders_ = nullptr;
  Logger logger_ = nullptr;
  Writer writer_ = nullptr;
  std::shared_ptr<MathLoggerDriver> math_log_driver_;
  BENDERSMETHOD method_ = BENDERSMETHOD::BENDERS;
  std::string context_ = bendersmethod_to_string(BENDERSMETHOD::BENDERS);
  std::string positive_unsupplied_file_;
  static constexpr const char* const LOLD_FILE = "LOLD.txt";

  [[nodiscard]] int RunExternalLoop();
  [[nodiscard]] int RunBenders();
  [[nodiscard]] std::shared_ptr<MathLoggerDriver> BuildMathLogger(
      bool benders_log_console) const;
  void PrepareForExecution(bool outer_loop);
  [[nodiscard]] std::variant<Benders::Criterion::CriterionInputData,
                             Benders::Criterion::OuterLoopCriterionInputData>
  ProcessCriterionInput();

  Benders::Criterion::CriterionInputData BuildPatternsUsingAreaFile();
  std::set<std::string> ReadAreaFile();
  void StartMessage();
  void EndMessage(const double execution_time);
  void AddCriterionOutputs();
  bool isCriterionListEmpty() const;
  void SetupLoggerAndOutputWriter(const BendersBaseOptions& benders_options);
  void ConfigureBenders(const BendersBaseOptions& benders_options,
                        const CouplingMap& coupling_map);
  void ConfigureSolverLog();

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