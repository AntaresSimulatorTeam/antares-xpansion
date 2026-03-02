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
    std::vector<char> result = AntaresProblemToXpansionProblemTranslator::convertSignToLEG(signs);
    ASSERT_EQ(result, expected);
}

// Fail test
TEST(AntaresProblemToXpansionProblemTranslatorTest, convertSignToLEGSpanFailWithInvalidChar)
{
    std::vector<char> signs = {'<', '=', 'a'};
    ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(signs),
                 std::runtime_error);
}

//'\0' in vector is error
TEST(AntaresProblemToXpansionProblemTranslatorTest, NullCharIsInvalid)
{
    std::vector<char> signs = {'<', '=', '\0'};
    ASSERT_THROW(AntaresProblemToXpansionProblemTranslator::convertSignToLEG(signs),
                 std::runtime_error);
}

TEST(RenameWeekNames, HourWeek1)
{
    std::vector<std::string> vnames = {"vhour<1>"};
    std::vector<std::string> cnames = {"chour<1>"};
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(1, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("vhour<1>"));
    ASSERT_EQ(constraints.at(0), std::string("chour<1>"));
}

TEST(RenameWeekNames, HourWeek2)
{
    std::vector<std::string> vnames = {"hour<1>"};
    std::vector<std::string> cnames; // vide, comme dans HourWeek1
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(2, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("hour<169>")); // 168 + 1
}

TEST(RenameWeekNames, DayWeek3)
{
    std::vector<std::string> vnames = {"day<3>"};
    std::vector<std::string> cnames;
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(3, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("day<17>")); // (3-1)*7 + 3 = 17
}

TEST(RenameWeekNames, WeekTokenNormalized)
{
    std::vector<std::string> vnames = {"week<5>"};
    std::vector<std::string> cnames;
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(3, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("week<2>")); // week normalisé à l'index 0
}

TEST(RenameWeekNames, OnlyFirstOccurrenceReplaced)
{
    std::vector<std::string> vnames = {"hour<1>_hour<2>"};
    std::vector<std::string> cnames;
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(2, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("hour<169>_hour<2>"));
}

TEST(RenameWeekNames, MultipleNames)
{
    std::vector<std::string> vnames = {"hour<1>", "day<1>", "week<1>"};
    std::vector<std::string> cnames = {"hour<1>", "day<1>", "week<1>"};
    RenameUtils rename_utils;
    auto result = rename_utils.rename_week_names(2, vnames, cnames, nullptr);
    ASSERT_TRUE(result.has_value());
    const auto& [variables, constraints] = *result;
    ASSERT_EQ(variables.at(0), std::string("hour<169>"));
    ASSERT_EQ(variables.at(1), std::string("day<8>"));
    ASSERT_EQ(variables.at(2), std::string("week<1>"));
    ASSERT_EQ(constraints.at(0), std::string("hour<169>"));
    ASSERT_EQ(constraints.at(1), std::string("day<8>"));
    ASSERT_EQ(constraints.at(2), std::string("week<1>"));
}

TEST(RenameWeekNames, NoTokenReturnsNullopt)
{
    std::vector<std::string> vnames = {"foo"};
    std::vector<std::string> cnames;
    auto result = RenameUtils().rename_week_names(1, vnames, cnames, nullptr);
    EXPECT_FALSE(result.has_value());
}

TEST(RenameWeekNames, WeekZeroReturnsNullopt)
{
    std::vector<std::string> vnames = {"hour<1>"};
    std::vector<std::string> cnames;
    auto result = RenameUtils().rename_week_names(0, vnames, cnames, nullptr);
    EXPECT_FALSE(result.has_value());
}
