#include <antares-xpansion/lpnamer/problem_modifier/RenameUtils.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"
#include "gtest/gtest.h"

TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGSpan)
{
    std::vector<char> signs = {'<', '=', '>'};
    std::vector<char> expected = {'L', 'E', 'G'};
    std::vector<char> result = AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
      std::span<char>(signs.data(), signs.size()));
    ASSERT_EQ(result, expected);
}

// Fail test
TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGSpanFailWithInvalidChar)
{
    std::vector<char> signs = {'<', '=', 'a'};
    ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
                   std::span<char>(signs.data(), signs.size())),
                 std::runtime_error);
}

//'\0' in vector is error
TEST(AntaresProblemToXpansionProblemTranslatorTest, NullCharIsInvalid)
{
    std::vector<char> signs = {'<', '=', '\0'};
    ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
                   std::span<char>(signs.data(), signs.size())),
                 std::runtime_error);
}

TEST(RenameWeekNames, HourWeek1)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"hour<1>"};
    RenameUtils::rename_week_names(1, names, container);
    ASSERT_EQ(container.at(1).size(), 1);
    EXPECT_EQ(container.at(1)[0], "hour<1>");
}

TEST(RenameWeekNames, HourWeek2)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"hour<1>"};
    RenameUtils::rename_week_names(2, names, container);
    ASSERT_EQ(container.at(2).size(), 1);
    EXPECT_EQ(container.at(2)[0], "hour<169>"); // 168 + 1
}

TEST(RenameWeekNames, DayWeek3)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"day<3>"};
    RenameUtils::rename_week_names(3, names, container);
    ASSERT_EQ(container.at(3).size(), 1);
    EXPECT_EQ(container.at(3)[0], "day<17>"); // (3-1)*7 + 3 = 17
}

TEST(RenameWeekNames, WeekTokenNormalized)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"week<5>"};
    RenameUtils::rename_week_names(3, names, container);
    ASSERT_EQ(container.at(3).size(), 1);
    EXPECT_EQ(container.at(3)[0], "week<2>"); // week normalized to 0-based index
}

TEST(RenameWeekNames, OnlyFirstOccurrenceReplaced)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"hour<1>_hour<2>"};
    RenameUtils::rename_week_names(2, names, container);
    ASSERT_EQ(container.at(2).size(), 1);
    EXPECT_EQ(container.at(2)[0], "hour<169>_hour<2>");
}

TEST(RenameWeekNames, NoTokenThrows)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"foo"};
    EXPECT_THROW(RenameUtils::rename_week_names(1, names, container), std::runtime_error);
}

TEST(RenameWeekNames, WeekZeroThrows)
{
    std::unordered_map<int, std::vector<std::string>> container;
    std::vector<std::string> names = {"hour<1>"};
    EXPECT_THROW(RenameUtils::rename_week_names(0, names, container), std::invalid_argument);
}
