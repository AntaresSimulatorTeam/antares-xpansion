#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"

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
class MasterGenerationExtExpectationTest
    : public MasterGenerationTest,
      public ::testing::WithParamInterface<Expectation> {};

class MasterGenerationSaveModeTest
    : public MasterGenerationTest,
      public ::testing::WithParamInterface<MasterGeneration::SaveMode> {};

// Test that master.mps is generated
TEST_P(MasterGenerationExtExpectationTest, master_file_is_generated) {
  auto [value, ext_expectation] = GetParam();
  MasterGeneration master_generation(temp_test_dir, active_links_,
                                     additionalConstraints_, couplings_,
                                     "master_formulation", "XPRESS", nullptr,
                                     solver_log_manager_, value);
  auto master_file = temp_test_dir / "lp" / "master";
  master_file.replace_extension(ext_expectation);
  ASSERT_TRUE(std::filesystem::exists(master_file));
}
INSTANTIATE_TEST_SUITE_P(
    Test_save_modes_master_generation, MasterGenerationExtExpectationTest,
    ::testing::Values(Expectation{MasterGeneration::SaveMode::MPS, "mps"},
                      Expectation{MasterGeneration::SaveMode::SAVE, "svf"}));

// Structure file is written
TEST_P(MasterGenerationSaveModeTest, structure_file_is_written) {
  MasterGeneration master_generation(
      temp_test_dir, active_links_, additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      GetParam());
  ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "structure.txt"));
}
INSTANTIATE_TEST_SUITE_P(
    Test_save_modes_master_generation, MasterGenerationSaveModeTest,
    ::testing::Values(MasterGeneration::SaveMode::MPS,
                      MasterGeneration::SaveMode::SAVE));

// Structure file contains master name
TEST_P(MasterGenerationSaveModeTest, structure_file_contains_master_name) {
  AddCandidate("dummy_candidate");
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