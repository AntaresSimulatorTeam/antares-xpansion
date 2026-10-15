#include <sstream>

#include "antares-xpansion/core/ResumeMode.h"
#include "gtest/gtest.h"

TEST(ResumeModeTest, IsResumeOnlyForResume)
{
    EXPECT_FALSE(IsResume(ResumeMode::COLD_START));
    EXPECT_TRUE(IsResume(ResumeMode::RESUME));
    EXPECT_FALSE(IsResume(ResumeMode::HOT_START));
}

TEST(ResumeModeTest, MasterHasAlphaVariablesExceptForColdStart)
{
    EXPECT_FALSE(MasterHasAlphaVariables(ResumeMode::COLD_START));
    EXPECT_TRUE(MasterHasAlphaVariables(ResumeMode::RESUME));
    EXPECT_TRUE(MasterHasAlphaVariables(ResumeMode::HOT_START));
}

TEST(ResumeModeTest, FromString)
{
    EXPECT_EQ(resumeModeFromString("cold_start"), ResumeMode::COLD_START);
    EXPECT_EQ(resumeModeFromString("resume"), ResumeMode::RESUME);
    EXPECT_EQ(resumeModeFromString("hot_start"), ResumeMode::HOT_START);
    EXPECT_THROW(resumeModeFromString("unknown"), std::runtime_error);
}

TEST(ResumeModeTest, StreamOperator)
{
    std::ostringstream os;
    os << ResumeMode::COLD_START << " " << ResumeMode::RESUME << " " << ResumeMode::HOT_START;
    EXPECT_EQ(os.str(), "cold_start resume hot_start");
}
