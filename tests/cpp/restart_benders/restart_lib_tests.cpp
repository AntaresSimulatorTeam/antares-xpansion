
#include "JsonWriter.h"
#include "LastIterationReader.h"
#include "core/ILogger.h"
#include "gtest/gtest.h"

const LogPoint x0 = {{"candidate1", 568.e6}, {"candidate2", 682e9}};
const LogPoint min_invest = {{"candidate1", 68.e6}, {"candidate2", 62e9}};
const LogPoint max_invest = {{"candidate1", 1568.e6}, {"candidate2", 3682e9}};
const LogData best_iteration_data = {
    15e5,       255e6,      200e6, 63,    63,   587e8, 8562e8, x0,
    min_invest, max_invest, 9e9,   3e-15, 5876, 999,   898};
const LogData last_iteration_data = {
    1e5,        255e6,      200e6, 159,   63,   587e8, 8562e8, x0,
    min_invest, max_invest, 9e9,   3e-15, 5876, 9999,  898};

class LastIterationReaderTest : public ::testing::Test {
 public:
  LastIterationReaderTest() = default;

  const std::filesystem::path _last_iteration_file = std::tmpnam(nullptr);
};

TEST_F(LastIterationReaderTest, ShouldFailIfInvalidFileIsGiven) {
  const auto delimiter = std::string("\n");
  const std::string expectedErrorString =
      delimiter + std::string("Invalid Json file: ") +
      _last_iteration_file.c_str() + delimiter;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto last_ite_reader = LastIterationReader(_last_iteration_file);
  std::cerr.rdbuf(initialBufferCerr);
  auto err_str = redirectedErrorStream.str();
  auto first_line_err =
      err_str.substr(0, err_str.find(delimiter, delimiter.length()) + 1);
  ASSERT_EQ(first_line_err, expectedErrorString);
}
void add_iteration(Json::Value& json_object, const std::string& iteration_name,
                   const LogData& iteration_data) {
  json_object[iteration_name]["lb"] = iteration_data.lb;
  json_object[iteration_name]["ub"] = iteration_data.ub;
  json_object[iteration_name]["best_ub"] = iteration_data.best_ub;
  json_object[iteration_name]["it"] = iteration_data.it;
  json_object[iteration_name]["best_it"] = iteration_data.best_it;
  json_object[iteration_name]["subproblem_cost"] =
      iteration_data.subproblem_cost;
  json_object[iteration_name]["invest_cost"] = iteration_data.invest_cost;

  Json::Value vectCandidates_l(Json::arrayValue);
  for (const auto& [candidate_name, value] : iteration_data.x0) {
    Json::Value candidate_l;
    candidate_l["candidate"] = candidate_name;
    candidate_l["invest"] = value;
    candidate_l["invest_min"] = iteration_data.min_invest.at(candidate_name);
    candidate_l["invest_max"] = iteration_data.max_invest.at(candidate_name);
    vectCandidates_l.append(candidate_l);
  }
  json_object[iteration_name]["candidates"] = vectCandidates_l;
  json_object[iteration_name]["optimality_gap"] = iteration_data.optimality_gap;
  json_object[iteration_name]["relative_gap"] = iteration_data.relative_gap;
  json_object[iteration_name]["max_iterations"] = iteration_data.max_iterations;
  json_object[iteration_name]["benders_elapsed_time"] =
      iteration_data.benders_elapsed_time;
  json_object[iteration_name]["duration"] = iteration_data.master_time;
}
TEST_F(LastIterationReaderTest, ShouldReturnLastAndBestIterations) {
  Json::Value json_content;
  add_iteration(json_content, "last", last_iteration_data);
  add_iteration(json_content, "best_iteration", best_iteration_data);
  std::ofstream jsonOut_l(_last_iteration_file);
  if (jsonOut_l) {
    // Output
    jsonOut_l << json_content << std::endl;
  } else {
    std::cerr << "Impossible d'ouvrir le fichier json " << _last_iteration_file
              << std::endl;
  }
  jsonOut_l.close();

  auto last_ite_reader = LastIterationReader(_last_iteration_file);
  const auto [last, best] = last_ite_reader.last_iteration_data();
  ASSERT_EQ(last, last_iteration_data);
  ASSERT_EQ(best, best_iteration_data);
}