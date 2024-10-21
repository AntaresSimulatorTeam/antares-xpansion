#include <json/json.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "antares-xpansion/sensitivity/SensitivityWriter.h"
#include "gtest/gtest.h"

const std::string peak_name = "peak";
const std::string semibase_name = "semibase";

class SensitivityWriterTest : public ::testing::Test {
 public:
  std::filesystem::path _fileName;

  void SetUp() override { _fileName = std::tmpnam(nullptr); }

  void TearDown() override { std::remove(_fileName.string().c_str()); }

  void verify_candidates_writing(const Point &candidates,
                                 const Json::Value &written_candidates) {
    ASSERT_EQ(candidates.size(), written_candidates.size());

    int candidate_id = 0;
    for (auto &kvp : candidates) {
      EXPECT_EQ(kvp.first, written_candidates[candidate_id][NAME_C].asString());
      EXPECT_DOUBLE_EQ(kvp.second,
                       written_candidates[candidate_id][INVEST_C].asDouble());
      candidate_id++;
    }
  }

  void verify_pbs_data_writing(const std::vector<SinglePbData> &pbs_data,
                               const Json::Value &written_pbs_data) {
    ASSERT_EQ(pbs_data.size(), written_pbs_data.size());

    for (int pb_id(0); pb_id < pbs_data.size(); pb_id++) {
      std::string pb_description = pbs_data[pb_id].str_pb_type;
      if (pbs_data[pb_id].pb_type == SensitivityPbType::PROJECTION) {
        pb_description += " " + pbs_data[pb_id].candidate_name;
      }
      EXPECT_EQ(pb_description, written_pbs_data[pb_id][PB_TYPE_C].asString());
      EXPECT_EQ(pbs_data[pb_id].opt_dir,
                written_pbs_data[pb_id][OPT_DIR_C].asString());
      EXPECT_EQ(pbs_data[pb_id].solver_status,
                written_pbs_data[pb_id][STATUS_C].asInt());
      EXPECT_DOUBLE_EQ(pbs_data[pb_id].objective,
                       written_pbs_data[pb_id][OBJECTIVE_C].asDouble());
      EXPECT_DOUBLE_EQ(pbs_data[pb_id].system_cost,
                       written_pbs_data[pb_id][SYSTEM_COST_C].asDouble());

      verify_candidates_writing(pbs_data[pb_id].candidates,
                                written_pbs_data[pb_id][CANDIDATES_C]);
    }
  }

  void verify_bounds_writing(
      const std::map<std::string, std::pair<double, double>> &candidates_bounds,
      const Json::Value &written_candidates_bounds) {
    ASSERT_EQ(candidates_bounds.size(), written_candidates_bounds.size());

    int candidate_id = 0;
    for (auto &kvp : candidates_bounds) {
      EXPECT_EQ(kvp.first,
                written_candidates_bounds[candidate_id][NAME_C].asString());
      EXPECT_EQ(kvp.second.first,
                written_candidates_bounds[candidate_id][LB_C].asDouble());
      EXPECT_EQ(kvp.second.second,
                written_candidates_bounds[candidate_id][UB_C].asDouble());
      candidate_id++;
    }
  }

  void verify_output_writing(const SensitivityInputData &input_data,
                             const std::vector<SinglePbData> &pbs_data,
                             Json::Value written_data) {
    EXPECT_EQ(input_data.epsilon, written_data[EPSILON_C].asDouble());
    EXPECT_EQ(input_data.best_ub,
              written_data[BEST_BENDERS_C].asDouble());

    verify_bounds_writing(input_data.candidates_bounds,
                          written_data[BOUNDS_C]);

    verify_pbs_data_writing(pbs_data,
                            written_data[SENSITIVITY_SOLUTION_C]);
  }
};

TEST_F(SensitivityWriterTest, GenerateAValidFile) {
  auto writer = SensitivityWriter(_fileName);
  writer.end_writing({}, {});

  std::ifstream fileStream(_fileName);
  fileStream.close();
  EXPECT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::filesystem::path &file_name) {
  Json::Value _input;
  std::ifstream input_file_l(file_name, std::ifstream::binary);
  Json::CharReaderBuilder builder_l;
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &_input, &errs)) {
    throw std::runtime_error("");
  }
  return _input;
}

TEST_F(SensitivityWriterTest, EndWritingPrintsOutputData) {
  auto writer = SensitivityWriter(_fileName);

  SensitivityInputData input_data;
  std::vector<SinglePbData> pbs_data;
  SinglePbData capex_min_data = {SensitivityPbType::CAPEX,
                                 CAPEX_C,
                                 "",
                                 MIN_C,
                                 10,
                                 100,
                                 {{peak_name, 20}, {semibase_name, 50}},
                                 0};
  SinglePbData proj_max_data = {SensitivityPbType::PROJECTION,
                                PROJECTION_C,
                                "some_candidate",
                                MAX_C,
                                200,
                                500,
                                {{peak_name, 10}, {semibase_name, 25}},
                                1};

  input_data.epsilon = 2;
  input_data.best_ub = 100;
  input_data.candidates_bounds = {{"my_cand", {12, 37}}, {"mock", {123, 456}}};
  pbs_data = {capex_min_data, proj_max_data};

  writer.end_writing(input_data, pbs_data);

  Json::Value json_content = get_value_from_json(_fileName);

  verify_output_writing(input_data, pbs_data, json_content);
}