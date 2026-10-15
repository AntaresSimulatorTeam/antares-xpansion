#include <sstream>

#include "antares-xpansion/core/RestartMode.h"
#include "gtest/gtest.h"

TEST(RestartModeTest, IsResumeOnlyForResume)
{
    EXPECT_FALSE(IsResume(RestartMode::COLD_START));
    EXPECT_TRUE(IsResume(RestartMode::RESUME));
    EXPECT_FALSE(IsResume(RestartMode::HOT_START));
}

TEST(RestartModeTest, MasterHasAlphaVariablesExceptForColdStart)
{
    EXPECT_FALSE(MasterHasAlphaVariables(RestartMode::COLD_START));
    EXPECT_TRUE(MasterHasAlphaVariables(RestartMode::RESUME));
    EXPECT_TRUE(MasterHasAlphaVariables(RestartMode::HOT_START));
}

TEST(RestartModeTest, FromString)
{
    EXPECT_EQ(restartModeFromString("cold_start"), RestartMode::COLD_START);
    EXPECT_EQ(restartModeFromString("resume"), RestartMode::RESUME);
    EXPECT_EQ(restartModeFromString("hot_start"), RestartMode::HOT_START);
    EXPECT_THROW(restartModeFromString("unknown"), std::runtime_error);
}

TEST(RestartModeTest, StreamOperator)
{
    std::ostringstream os;
    os << RestartMode::COLD_START << " " << RestartMode::RESUME << " " << RestartMode::HOT_START;
    EXPECT_EQ(os.str(), "cold_start resume hot_start");
}
