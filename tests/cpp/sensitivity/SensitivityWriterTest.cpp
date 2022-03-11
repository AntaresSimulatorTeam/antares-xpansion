#include <json/json.h>

#include <cstdio>
#include <fstream>

#include "SensitivityWriter.h"
#include "gtest/gtest.h"

const std::string peak_name = "peak";
const std::string semibase_name = "semibase";

class SensitivityWriterTest : public ::testing::Test {
 public:
  std::string _fileName;

  void SetUp() override { _fileName = std::tmpnam(nullptr); }

  void TearDown() override { std::remove(_fileName.c_str()); }

  void verify_candidates_writing(const Point &candidates,
                                 const Json::Value &written_candidates) {
    ASSERT_EQ(candidates.size(), written_candidates.size());

    int candidate_id = 0;
    for (auto &kvp : candidates) {
      ASSERT_EQ(kvp.first, written_candidates[candidate_id][NAME_C].asString());
      ASSERT_DOUBLE_EQ(kvp.second,
                       written_candidates[candidate_id][INVEST_C].asDouble());
      candidate_id++;
    }
  }

  void verify_pbs_data_writing(const std::vector<SinglePbData> &pbs_data,
                               const Json::Value &written_pbs_data) {
    ASSERT_EQ(pbs_data.size(), written_pbs_data.size());

    for (int pb_id(0); pb_id < pbs_data.size(); pb_id++) {
      ASSERT_EQ(pbs_data[pb_id].str_pb_type,
                written_pbs_data[pb_id][PB_TYPE_C].asString());
      ASSERT_EQ(pbs_data[pb_id].opt_dir,
                written_pbs_data[pb_id][OPT_DIR_C].asString());
      ASSERT_EQ(pbs_data[pb_id].solver_status,
                written_pbs_data[pb_id][STATUS_C].asInt());
      ASSERT_DOUBLE_EQ(pbs_data[pb_id].objective,
                       written_pbs_data[pb_id][OBJECTIVE_C].asDouble());
      ASSERT_DOUBLE_EQ(pbs_data[pb_id].system_cost,
                       written_pbs_data[pb_id][SYSTEM_COST_C].asDouble());

      verify_candidates_writing(pbs_data[pb_id].candidates,
                                written_pbs_data[pb_id][CANDIDATES_C]);
    }
  }

  void verify_output_writing(const SensitivityOutputData &output_data,
                             Json::Value written_data) {
    ASSERT_EQ(output_data.epsilon, written_data[EPSILON_C].asDouble());
    ASSERT_EQ(output_data.best_benders_cost,
              written_data[BEST_BENDERS_C].asDouble());

    verify_pbs_data_writing(output_data.pbs_data,
                            written_data[SENSITIVITY_SOLUTION_C]);
  }
};

TEST_F(SensitivityWriterTest, GenerateAValidFile) {
  auto writer = SensitivityWriter(_fileName);
  writer.end_writing({});

  std::ifstream fileStream(_fileName);
  fileStream.close();
  ASSERT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::string &file_name) {
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

  SensitivityOutputData output_data;
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

  output_data.epsilon = 2;
  output_data.best_benders_cost = 100;
  output_data.pbs_data = {capex_min_data, proj_max_data};

  writer.end_writing(output_data);

  Json::Value json_content = get_value_from_json(_fileName);

  verify_output_writing(output_data, json_content);
}