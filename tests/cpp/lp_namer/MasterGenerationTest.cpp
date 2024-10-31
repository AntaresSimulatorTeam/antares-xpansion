#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"

//Noop ProblemGenerationLogger
class NoopProblemGenerationLogger : public ProblemGenerationLog::ProblemGenerationLogger {
 public:
  using ProblemGenerationLogger::ProblemGenerationLogger;
  void display_message(const std::string &message) override {}
  void display_message(const std::string &message, const LogUtils::LOGLEVEL log_level,
                       const std::string &context) override {}
  void PrintIterationSeparatorBegin() override {}
  void PrintIterationSeparatorEnd() override {}

};


//Fixture
class MasterGenerationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a temporary directory
    temp_test_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
    std::filesystem::create_directories(temp_test_dir / "lp");
  }

  std::filesystem::path temp_test_dir;
  AdditionalConstraints additionalConstraints_{nullptr};
  Couplings couplings_;
  SolverLogManager solver_log_manager_;
};

using Expectation = std::pair<MasterGeneration::SaveMode, std::string>;
class MasterGenerationSaveModeTest : public MasterGenerationTest,
                                     public ::testing::WithParamInterface<Expectation> {};

// Test that master.mps is generated
TEST_P(MasterGenerationSaveModeTest, master_file_is_generated) {
  auto [value, ext_expectation] = GetParam();
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      value);
  auto master_file = temp_test_dir / "lp" / "master";
  master_file.replace_extension(ext_expectation);
  ASSERT_TRUE(std::filesystem::exists(master_file));
}
INSTANTIATE_TEST_SUITE_P(Test_save_modes_master_generation,
                         MasterGenerationSaveModeTest,
                         ::testing::Values(Expectation{MasterGeneration::SaveMode::MPS, "mps"}, Expectation {MasterGeneration::SaveMode::SAVE, "svf"}));

//Structure file is written
TEST_F(MasterGenerationTest, structure_file_is_written) {
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      MasterGeneration::SaveMode::MPS);
  ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "structure.txt"));
}

//Structure file contains master name
TEST_F(MasterGenerationTest, structure_file_contains_master_name) {
  CandidateData candidate_data;
  candidate_data.name = "dummy_candidate";

  ActiveLinksBuilder active_link_builder({candidate_data}, {}, std::make_shared<NoopProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO));

  MasterGeneration master_generation(
      temp_test_dir, active_link_builder.getLinks(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      MasterGeneration::SaveMode::MPS);
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

////Structure file contains master name without file extension
TEST_F(MasterGenerationTest, structure_file_contains_master_name_without_extension) {
  CandidateData candidate_data;
  candidate_data.name = "dummy_candidate";

  ActiveLinksBuilder active_link_builder({candidate_data}, {}, std::make_shared<NoopProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO));

  MasterGeneration master_generation(
      temp_test_dir, active_link_builder.getLinks(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      MasterGeneration::SaveMode::MPS);
  std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
  std::string line;
  bool found = false;
  while (std::getline(structure_file, line)) {
    auto pos = line.find("master");
    if (pos != std::string::npos) {
      auto with_ext_pos = line.find("master.mps");
      if (with_ext_pos == pos) {
        continue;
      }
      found = true;
      break;
    }
  }
  ASSERT_TRUE(found);
}