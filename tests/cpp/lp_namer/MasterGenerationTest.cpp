#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"

//Fixture
class MasterGenerationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a temporary directory
    temp_test_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
    std::filesystem::create_directories(temp_test_dir / "lp");
  }

  std::filesystem::path temp_test_dir;
};

// Test that master.mps is generated
TEST_F(MasterGenerationTest, master_mps_is_generated) {
  AdditionalConstraints additionalConstraints_p(nullptr);
  Couplings couplings;
  SolverLogManager solver_log_manager;
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_p,
      couplings, "master_formulation", "XPRESS", nullptr, solver_log_manager,
      MasterGeneration::SaveMode::MPS);
  assert(std::filesystem::exists(temp_test_dir / "lp" / "master.mps"));
}

// Test that master.svf is generated
TEST_F(MasterGenerationTest, master_sfx_is_generated) {
  AdditionalConstraints additionalConstraints_p(nullptr);
  Couplings couplings;
  SolverLogManager solver_log_manager;
  MasterGeneration master_generation(
      temp_test_dir, std::vector<ActiveLink>(), additionalConstraints_p,
      couplings, "master_formulation", "XPRESS", nullptr, solver_log_manager,
      MasterGeneration::SaveMode::SAVE);
  assert(std::filesystem::exists(temp_test_dir / "lp" / "master.svf"));
}