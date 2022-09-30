#include <vector>

#include "OptionsParser.h"
#include "gtest/gtest.h"
class OptionsParserTest : public ::testing::Test {
 protected:
  OptionsParser options_parser;
};
TEST_F(OptionsParserTest, FailsWithArgcEqualToZero) {
  char c = 'c';
  std::vector<char*> argv = {&c};
  try {
    options_parser.Parse(0, argv.data());
    FAIL() << "Expected OptionsParser::InvalidNumberOfArgumentsPassedToParser";
  } catch (const OptionsParser::InvalidNumberOfArgumentsPassedToParser& err) {
    EXPECT_EQ(err.what(), std::string("Error while parsing ") +
                              +
                             ": invalid number arguments:  0");
  } catch (...) {
    FAIL() << "Expected OptionsParser::InvalidNumberOfArgumentsPassedToParser";
  }
}