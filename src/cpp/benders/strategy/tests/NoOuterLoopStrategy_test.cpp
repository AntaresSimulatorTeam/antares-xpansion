#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/strategy/NoOuterLoopStrategy.h"

/**
 * @brief Test fixture for NoOuterLoopStrategy tests
 */
class NoOuterLoopStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        strategy_ = std::make_unique<NoOuterLoopStrategy>();
    }

    std::unique_ptr<NoOuterLoopStrategy> strategy_;
};

/**
 * @brief Test that Run is a no-op
 */
TEST_F(NoOuterLoopStrategyTest, Run)
{
    EXPECT_NO_THROW(strategy_->Run());
    
    // Can be called multiple times
    EXPECT_NO_THROW(strategy_->Run());
    EXPECT_NO_THROW(strategy_->Run());
}

/**
 * @brief Test that RunAttachedAlgo is a no-op
 */
TEST_F(NoOuterLoopStrategyTest, RunAttachedAlgo)
{
    EXPECT_NO_THROW(strategy_->RunAttachedAlgo());
    EXPECT_NO_THROW(strategy_->RunAttachedAlgo());
}

/**
 * @brief Test that UpdateMaster always returns false
 */
TEST_F(NoOuterLoopStrategyTest, UpdateMaster)
{
    EXPECT_FALSE(strategy_->UpdateMaster());
    
    // Even after multiple calls, still false
    strategy_->Run();
    EXPECT_FALSE(strategy_->UpdateMaster());
}

/**
 * @brief Test that PrintLog is a no-op
 */
TEST_F(NoOuterLoopStrategyTest, PrintLog)
{
    EXPECT_NO_THROW(strategy_->PrintLog());
    EXPECT_NO_THROW(strategy_->PrintLog());
}

/**
 * @brief Test that init_data is a no-op
 */
TEST_F(NoOuterLoopStrategyTest, InitData)
{
    EXPECT_NO_THROW(strategy_->init_data());
    EXPECT_NO_THROW(strategy_->init_data());
}

/**
 * @brief Test that isExceptionRaised always returns false
 */
TEST_F(NoOuterLoopStrategyTest, IsExceptionRaised)
{
    EXPECT_FALSE(strategy_->isExceptionRaised());
    
    // Even after running, no exceptions
    strategy_->Run();
    EXPECT_FALSE(strategy_->isExceptionRaised());
}

/**
 * @brief Test that lambda values are default (0.0)
 */
TEST_F(NoOuterLoopStrategyTest, LambdaValues)
{
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMin(), 0.0);
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMax(), 0.0);
}

/**
 * @brief Test that feasibility check is a no-op
 */
TEST_F(NoOuterLoopStrategyTest, CheckFeasibility)
{
    EXPECT_NO_THROW(strategy_->OuterLoopCheckFeasibility());
    EXPECT_NO_THROW(strategy_->OuterLoopCheckFeasibility());
}

/**
 * @brief Test that bilevel checks are no-ops
 */
TEST_F(NoOuterLoopStrategyTest, BilevelChecks)
{
    EXPECT_NO_THROW(strategy_->OuterLoopBilevelChecks());
    EXPECT_NO_THROW(strategy_->OuterLoopBilevelChecks());
}

/**
 * @brief Test complete workflow (all no-ops)
 */
TEST_F(NoOuterLoopStrategyTest, CompleteWorkflow)
{
    EXPECT_NO_THROW({
        strategy_->init_data();
        strategy_->Run();
        strategy_->RunAttachedAlgo();
        EXPECT_FALSE(strategy_->UpdateMaster());
        strategy_->OuterLoopCheckFeasibility();
        strategy_->OuterLoopBilevelChecks();
        strategy_->PrintLog();
        EXPECT_FALSE(strategy_->isExceptionRaised());
    });
}

/**
 * @brief Test multiple instances don't interfere
 */
TEST(NoOuterLoopStrategyMultiInstanceTest, MultipleInstances)
{
    auto strategy1 = std::make_unique<NoOuterLoopStrategy>();
    auto strategy2 = std::make_unique<NoOuterLoopStrategy>();
    
    strategy1->Run();
    EXPECT_FALSE(strategy1->UpdateMaster());
    
    strategy2->init_data();
    EXPECT_FALSE(strategy2->isExceptionRaised());
    
    // Both should be independent
    EXPECT_FALSE(strategy1->isExceptionRaised());
    EXPECT_FALSE(strategy2->UpdateMaster());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
