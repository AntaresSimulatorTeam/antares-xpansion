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

// Test that master.mps is generated
TEST_F(MasterGenerationTest, master_mps_is_generated) {
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      MasterGeneration::SaveMode::MPS);
  ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "master.mps"));
}

// Test that master.svf is generated
TEST_F(MasterGenerationTest, master_sfx_is_generated) {
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_,
      couplings_, "master_formulation", "XPRESS", nullptr, solver_log_manager_,
      MasterGeneration::SaveMode::SAVE);
  ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "master.svf"));
}

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