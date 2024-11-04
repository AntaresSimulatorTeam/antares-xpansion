#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"
#include "antares-xpansion/multisolver_interface/Solver.h"

// Noop ProblemGenerationLogger
class NoopProblemGenerationLogger
    : public ProblemGenerationLog::ProblemGenerationLogger {
 public:
  using ProblemGenerationLogger::ProblemGenerationLogger;
  void display_message(const std::string &message) override {}
  void display_message(const std::string &message,
                       const LogUtils::LOGLEVEL log_level,
                       const std::string &context) override {}
  void PrintIterationSeparatorBegin() override {}
  void PrintIterationSeparatorEnd() override {}
};

// Fixture
class MasterGenerationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a temporary directory
    temp_test_dir =
        std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
    std::filesystem::create_directories(temp_test_dir / "lp");
  }

  void AddCandidate(std::string name) {
    CandidateData candidate_data;
    candidate_data.name = name;

    ActiveLinksBuilder active_link_builder(
        {candidate_data}, {},
        std::make_shared<NoopProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO));

    auto links = active_link_builder.getLinks();
    active_links_.insert(active_links_.end(), links.begin(),
                             links.end());
  }

  std::filesystem::path temp_test_dir;
  AdditionalConstraints additionalConstraints_{nullptr};
  Couplings couplings_;
  SolverLogManager solver_log_manager_;
  std::vector<ActiveLink> active_links_;
};

using Expectation = std::pair<MasterGeneration::SaveMode, std::string>;
using SolverAndExpectation = std::pair<std::string, Expectation>;
class MasterGenerationExtExpectationTest
    : public MasterGenerationTest,
      public ::testing::WithParamInterface<Expectation> {};

class MasterGenerationSaveModeTest
    : public MasterGenerationTest,
      public ::testing::WithParamInterface<MasterGeneration::SaveMode> {};

using SolverName = std::string;
class TestForSolverAndMode : public MasterGenerationTest,
                             public ::testing::WithParamInterface<std::tuple<SolverName, MasterGeneration::SaveMode>> {};

using SolverAndExpectationParams = std::pair<SolverName, Expectation>;
class TestForSolverAndExpectation : public MasterGenerationTest,
                                    public ::testing::WithParamInterface<SolverAndExpectationParams> {};
auto solverNamesGenerator = ::testing::ValuesIn(SolverLoader::GetSupportedSolvers());
auto solverAndSaveModeGenerator = ::testing::Combine(solverNamesGenerator, ::testing::Values(MasterGeneration::SaveMode::MPS, MasterGeneration::SaveMode::SAVE));

#define SKIP_UNAVAILABLE_SOLVER(solver_name) \
  auto available_solvers = SolverLoader::GetAvailableSolvers(std::make_shared<NoopProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO)); \
  if (std::ranges::find(available_solvers, solver_name) == available_solvers.end()) { \
    GTEST_SKIP() << "Solver " << solver_name << " is not available"; \
  }

TEST_P(TestForSolverAndExpectation, master_file_is_generated) {
  auto&& [solver_name, expectation] = GetParam();
  auto&& [value, ext_expectation] = expectation;
  SKIP_UNAVAILABLE_SOLVER(solver_name)
  AddCandidate("dummy_candidate");
  MasterGeneration master_generation(temp_test_dir, active_links_,
                                     additionalConstraints_, couplings_,
                                     "master_formulation", solver_name, nullptr,
                                     solver_log_manager_, value);
  auto master_file = temp_test_dir / "lp" / "master";
  master_file.replace_extension(ext_expectation);
  ASSERT_TRUE(std::filesystem::exists(master_file));
}
INSTANTIATE_TEST_SUITE_P(
    Test_save_modes_master_generation, TestForSolverAndExpectation,
    ::testing::Values(
        SolverAndExpectation{"XPRESS", Expectation{MasterGeneration::SaveMode::MPS, "mps"}},
        SolverAndExpectation{"XPRESS", Expectation{MasterGeneration::SaveMode::SAVE, "svf"}},
        SolverAndExpectation{"CBC", Expectation{MasterGeneration::SaveMode::MPS, "mps"}},
        SolverAndExpectation{"CBC", Expectation{MasterGeneration::SaveMode::SAVE, "mps"}},
        SolverAndExpectation{"CLP", Expectation{MasterGeneration::SaveMode::MPS, "mps"}},
        SolverAndExpectation{"CLP", Expectation{MasterGeneration::SaveMode::SAVE, "mps"}}));

// Structure file is written
TEST_P(TestForSolverAndMode, structure_file_is_written) {
  auto&& [solver_name, save_mode] = GetParam();
  SKIP_UNAVAILABLE_SOLVER(solver_name)
  AddCandidate("dummy_candidate");
  MasterGeneration master_generation(
      temp_test_dir, active_links_, additionalConstraints_,
      couplings_, "master_formulation", solver_name, nullptr, solver_log_manager_,
      save_mode);
  ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "structure.txt"));
}
INSTANTIATE_TEST_SUITE_P(
    Test_save_modes_master_generation, TestForSolverAndMode,
    solverAndSaveModeGenerator);


// Structure file contains master name
TEST_P(MasterGenerationSaveModeTest, structure_file_contains_master_name) {
  AddCandidate("dummy_candidate");
  //SKIP_UNAVAILABLE_SOLVER(solver_name)
  MasterGeneration master_generation(
      temp_test_dir, active_links_, additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      GetParam());
  std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
  std::string line;
  bool found = false;
  while (std::getline(structure_file, line)) {
    if (line.find("master") != std::string::npos) {
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
}
INSTANTIATE_TEST_SUITE_P(
    structure_file_contains_master_name, MasterGenerationSaveModeTest,
    ::testing::Values(MasterGeneration::SaveMode::MPS,
                      MasterGeneration::SaveMode::SAVE));

////Structure file contains master name without file extension
TEST_P(MasterGenerationExtExpectationTest,
       structure_file_contains_master_name_without_extension) {
  AddCandidate("dummy_candidate");
  auto [value, ext_expectation] = GetParam();
  //SKIP_UNAVAILABLE_SOLVER(solver_name)
  MasterGeneration master_generation(
      temp_test_dir, active_links_, additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      value);
  std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
  std::string line;
  bool found = false;
  while (std::getline(structure_file, line)) {
    auto pos = line.find("master");
    if (pos != std::string::npos) {
      auto with_ext_pos = line.find("master." + ext_expectation);
      if (with_ext_pos == pos) {
        continue;
      }
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
}
INSTANTIATE_TEST_SUITE_P(
    structure_file_contains_master_name_without_extension,
    MasterGenerationExtExpectationTest,
    ::testing::Values(Expectation{MasterGeneration::SaveMode::MPS, "mps"},
                      Expectation{MasterGeneration::SaveMode::SAVE, "svf"}));