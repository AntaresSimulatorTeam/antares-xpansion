#include "gtest/gtest.h"

#include "AntaresProblemToXpansionProblemTranslator.h"

//Test convert to leg
TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEG) {
  std::vector<char> signs = {'<', '=', '>', '\0'};
  std::vector<char> expected = {'L', 'E', 'G'};
  std::vector<char> result = AntaresProblemToXpansionProblemTranslator::convertSignToLEG(signs.data());
  ASSERT_EQ(result, expected);
}

//Fail case
TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGFail) {
  std::vector<char> signs = {'<', '=', '>', 'a', '\0'};
  ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(signs.data()), std::runtime_error);
}