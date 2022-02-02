#include "gtest/gtest.h"

#include "SensitivityWriter.h"
#include <cstdio>
#include <fstream>
#include <json/json.h>

class SensitivityWriterTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        _fileName = std::tmpnam(nullptr);
    }

    void TearDown() override
    {
        std::remove(_fileName.c_str());
    }

    std::string _fileName;
};

TEST_F(SensitivityWriterTest, GenerateAValidFile)
{
    auto writer = SensitivityWriter(_fileName);
    writer.dump();

    std::ifstream fileStream(_fileName);
    fileStream.close();
    ASSERT_TRUE(fileStream.good());
}

Json::Value get_value_from_json(const std::string &file_name)
{
    Json::Value _input;
    std::ifstream input_file_l(file_name, std::ifstream::binary);
    Json::CharReaderBuilder builder_l;
    std::string errs;
    if (!parseFromStream(builder_l, input_file_l, &_input, &errs))
    {
        throw std::runtime_error("");
    }
    return _input;
}

TEST_F(SensitivityWriterTest, EndWritingPrintsOutputData)
{
    auto writer = SensitivityWriter(_fileName);

    SensitivityOutputData output_data;

    output_data.epsilon = 2;
    output_data.best_benders_cost = 100;
    output_data.system_cost = 120;
    output_data.pb_objective = 80;
    output_data.candidates = {{"peak", 50}, {"semibase", 30}};
    output_data.pb_status = SOLVER_STATUS::OPTIMAL;

    writer.end_writing(output_data);

    Json::Value json_content = get_value_from_json(_fileName);

    ASSERT_EQ(output_data.epsilon, json_content["epsilon"].asDouble());
    ASSERT_EQ(output_data.best_benders_cost, json_content["best_benders_cost"].asDouble());
    ASSERT_EQ(output_data.system_cost, json_content["system_cost"].asDouble());
    ASSERT_EQ(output_data.pb_objective, json_content["pb_objective"].asDouble());
    ASSERT_EQ(output_data.pb_status, json_content["pb_status"].asInt());

    int candidate_number = 0;
    for (auto &kvp : output_data.candidates)
    {   
        ASSERT_EQ(kvp.first, json_content["sensitivity_solution_candidates"][candidate_number]["name"].asString());
        ASSERT_EQ(kvp.second, json_content["sensitivity_solution_candidates"][candidate_number]["invest"].asDouble());
        candidate_number++;
    }
}