#include "antares-xpansion/sensitivity/SensitivityInputReader.h"
#include "gtest/gtest.h"

class SensitivityInputReaderTest : public ::testing::Test {
 public:
  std::string data_test_dir;

  std::string last_master_mps_path;
  std::string basis_path;
  std::string json_input_path;
  std::string benders_output_path;
  std::string structure_path;

  const std::string peak_name = "peak";
  const std::string semibase_name = "semibase";

 protected:
  void SetUp() override {
    data_test_dir = "data_test/sensitivity";

    last_master_mps_path = data_test_dir + "/toy_last_iteration.mps";
    json_input_path = data_test_dir + "/sensitivity_in.json";
    benders_output_path = data_test_dir + "/benders_out.json";
    structure_path = data_test_dir + "/structure.txt";
    basis_path = data_test_dir + "/toy_basis.bss";
  }

  void TearDown() override {}

  void verify_input_data(const SensitivityInputData &input_data,
                         const SensitivityInputData &expec_input_data) {
    EXPECT_EQ(input_data.epsilon, expec_input_data.epsilon);
    EXPECT_EQ(input_data.best_ub, expec_input_data.best_ub);
    EXPECT_EQ(input_data.benders_capex, expec_input_data.benders_capex);
    ASSERT_EQ(input_data.benders_solution.size(),
              input_data.benders_solution.size());
    EXPECT_EQ(input_data.benders_solution, expec_input_data.benders_solution);

    EXPECT_EQ(input_data.name_to_id, expec_input_data.name_to_id);
    EXPECT_EQ(input_data.candidates_bounds, expec_input_data.candidates_bounds);
    EXPECT_EQ(input_data.capex, expec_input_data.capex);
    ASSERT_EQ(input_data.projection.size(), expec_input_data.projection.size());
    for (int i(0); i < input_data.projection.size(); i++) {
      EXPECT_EQ(input_data.projection[i], expec_input_data.projection[i]);
    }
  }
};

TEST_F(SensitivityInputReaderTest, GetInputData) {
  SensitivityInputReader input_reader(json_input_path, benders_output_path,
                                      last_master_mps_path, basis_path,
                                      structure_path);
  SensitivityInputData input_data = input_reader.get_input_data();

  SensitivityInputData expec_input_data = {
      1e4,
      2200,
      1000,
      {{peak_name, 15}, {semibase_name, 3}},
      {{peak_name, 8}, {semibase_name, 32}},
      nullptr,
      basis_path,
      {{peak_name, {0, 3000}}, {semibase_name, {0, 400}}},
      true,
      {peak_name, semibase_name}};

  verify_input_data(input_data, expec_input_data);
}