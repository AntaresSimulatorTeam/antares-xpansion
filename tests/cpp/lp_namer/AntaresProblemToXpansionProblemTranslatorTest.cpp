#include "gtest/gtest.h"

#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGSpan) {
  std::vector<char> signs = {'<', '=', '>'};
  std::vector<char> expected = {'L', 'E', 'G'};
  std::vector<char> result = AntaresProblemToXpansionProblemTranslator::convertSignToLEG(std::span<char>(signs.data(), signs.size()));
  ASSERT_EQ(result, expected);
}

//Fail test
TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGSpanFailWithInvalidChar) {
  std::vector<char> signs = {'<', '=', 'a'};
  ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(std::span<char>(signs.data(), signs.size())), std::runtime_error);
}

//'\0' in vector is error
TEST(AntaresProblemToXpansionProblemTranslatorTest, NullCharIsInvalid) {
  std::vector<char> signs = {'<', '=', '\0'};
  ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(std::span<char>(signs.data(), signs.size())), std::runtime_error);
}