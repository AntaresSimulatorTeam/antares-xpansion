#include <vector>

#include "antares-xpansion/helpers/OptionsParser.h"
#include "gtest/gtest.h"
namespace po = boost::program_options;

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
    EXPECT_EQ(err.ErrorMessage().c_str(),
              std::string("Error while parsing ") +
                  +": invalid number arguments:  0");
  } catch (...) {
    FAIL() << "Expected OptionsParser::InvalidNumberOfArgumentsPassedToParser";
  }
}

TEST_F(OptionsParserTest, FailsIfRequiredOptionIsAddedWithNullArgv) {
  int option;
  options_parser.AddOptions()("option,-o", po::value<int>(&option)->required(),
                              "test option");
  try {
    options_parser.Parse(1, nullptr);
  } catch (const OptionsParser::NullArgumentsValues& e) {
    EXPECT_EQ(
        e.ErrorMessage().c_str(),
        std::string("Error while parsing  options: null Arguments values!"));
  }
}
TEST_F(OptionsParserTest, FailsIfRequiredOptionIsAddedWithMissingOption) {
  int option;
  options_parser.AddOptions()("option,-o", po::value<int>(&option)->required(),
                              "test option");
  char c = 'c';
  char* pc = &c;
  char** ppc = &pc;
  try {
    options_parser.Parse(1, ppc);
  } catch (const std::exception& e) {
    EXPECT_EQ(e.what(),
              std::string("the option '--option' is required but missing"));
  }
}
