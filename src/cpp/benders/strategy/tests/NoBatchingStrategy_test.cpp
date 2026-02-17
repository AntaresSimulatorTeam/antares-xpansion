#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/strategy/NoBatchingStrategy.h"

/**
 * @brief Test fixture for NoBatchingStrategy tests
 */
class NoBatchingStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        strategy_ = std::make_unique<NoBatchingStrategy>();
    }

    std::unique_ptr<NoBatchingStrategy> strategy_;
};

/**
 * @brief Test that InitializeProblems is a no-op
 */
TEST_F(NoBatchingStrategyTest, InitializeProblems)
{
    // Should not crash
    EXPECT_NO_THROW(strategy_->InitializeProblems());
    
    // Can be called multiple times
    EXPECT_NO_THROW(strategy_->InitializeProblems());
    EXPECT_NO_THROW(strategy_->InitializeProblems());
}

/**
 * @brief Test that UpdateStoppingCriterion is a no-op
 */
TEST_F(NoBatchingStrategyTest, UpdateStoppingCriterion)
{
    // Should not crash
    EXPECT_NO_THROW(strategy_->UpdateStoppingCriterion());
    
    // Can be called multiple times
    EXPECT_NO_THROW(strategy_->UpdateStoppingCriterion());
    EXPECT_NO_THROW(strategy_->UpdateStoppingCriterion());
}

/**
 * @brief Test that ShouldRelaxationStop always returns false
 */
TEST_F(NoBatchingStrategyTest, ShouldRelaxationStop)
{
    // Should always return false (no batching means no batch-based stopping)
    EXPECT_FALSE(strategy_->ShouldRelaxationStop());
    
    // Even after updates, still false
    strategy_->UpdateStoppingCriterion();
    EXPECT_FALSE(strategy_->ShouldRelaxationStop());
    
    // After initialization, still false
    strategy_->InitializeProblems();
    EXPECT_FALSE(strategy_->ShouldRelaxationStop());
}

/**
 * @brief Test that strategy can be created and destroyed safely
 */
TEST(NoBatchingStrategyLifecycleTest, CreateAndDestroy)
{
    EXPECT_NO_THROW({
        auto strategy = std::make_unique<NoBatchingStrategy>();
        strategy->InitializeProblems();
        strategy->UpdateStoppingCriterion();
        EXPECT_FALSE(strategy->ShouldRelaxationStop());
    });
}

/**
 * @brief Test multiple instances don't interfere
 */
TEST(NoBatchingStrategyMultiInstanceTest, MultipleInstances)
{
    auto strategy1 = std::make_unique<NoBatchingStrategy>();
    auto strategy2 = std::make_unique<NoBatchingStrategy>();
    
    strategy1->InitializeProblems();
    EXPECT_FALSE(strategy1->ShouldRelaxationStop());
    
    strategy2->UpdateStoppingCriterion();
    EXPECT_FALSE(strategy2->ShouldRelaxationStop());
    
    // Both should be independent
    EXPECT_FALSE(strategy1->ShouldRelaxationStop());
    EXPECT_FALSE(strategy2->ShouldRelaxationStop());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
