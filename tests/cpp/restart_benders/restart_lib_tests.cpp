
#include "JsonWriter.h"
#include "LastIterationReader.h"
#include "LastIterationWriter.h"
#include "common.h"
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

  const std::string invalid_file_path = "";
  const std::filesystem::path _last_iteration_file = std::tmpnam(nullptr);
};

TEST_F(LastIterationReaderTest, ShouldFailIfInvalidFileIsGiven) {
  const auto delimiter = std::string("\n");
  std::stringstream expectedErrorString;
  expectedErrorString << delimiter << "Invalid Json file: " << invalid_file_path
                      << delimiter;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto last_ite_reader = LastIterationReader(invalid_file_path);
  std::cerr.rdbuf(initialBufferCerr);
  auto err_str = redirectedErrorStream.str();
  auto first_line_err =
      err_str.substr(0, err_str.find(delimiter, delimiter.length()) + 1);
  ASSERT_EQ(first_line_err, expectedErrorString.str());
}
TEST_F(LastIterationReaderTest,
       ReaderShouldReturnLastAndBestIterationsWrittenByWriter) {
  auto writer = LastIterationWriter(_last_iteration_file);
  writer.save_best_and_last_iterations(best_iteration_data,
                                       last_iteration_data);
  auto reader = LastIterationReader(_last_iteration_file);
  const auto [last, best] = reader.last_iteration_data();
  ASSERT_EQ(last, last_iteration_data);
  ASSERT_EQ(best, best_iteration_data);
}

class LastIterationWriterTest : public ::testing::Test {
 public:
  LastIterationWriterTest() = default;

  const std::string _invalid_file_path = "";
};
TEST_F(LastIterationWriterTest, ShouldFailIfInvalidFileIsGiven) {
  std::stringstream expectedErrorString;
  expectedErrorString << "Invalid Json file: " << _invalid_file_path
                      << std::endl;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto writer = LastIterationWriter(_invalid_file_path);
  writer.save_best_and_last_iterations(best_iteration_data,
                                       last_iteration_data);
  std::cerr.rdbuf(initialBufferCerr);
  ASSERT_EQ(redirectedErrorStream.str(), expectedErrorString.str());
}