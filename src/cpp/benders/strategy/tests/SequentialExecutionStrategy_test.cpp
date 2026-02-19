#include "antares-xpansion/benders/strategy/SequentialExecutionStrategy.h"

#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"

/**
 * @brief Mock BendersSequential for testing purposes
 *
 * Overrides BendersSequential methods to avoid actual execution
 * while allowing us to verify the adapter delegates correctly.
 */
class MockBendersSequential: public BendersSequential
{
public:
    MockBendersSequential():
        BendersSequential(BendersBaseOptions(SolverBaseOptions()),
                          build_void_logger(),
                          std::shared_ptr<Output::OutputWriter>(),
                          std::shared_ptr<MathLoggerDriver>()),
        launch_called_(false),
        init_problems_called_(false)
    {
    }

    void launch() override
    {
        launch_called_ = true;
    }

    void InitializeProblems() override
    {
        init_problems_called_ = true;
    }

    std::string BendersName() const override
    {
        return "MockSequential";
    }

    bool was_launch_called() const
    {
        return launch_called_;
    }

    bool was_init_problems_called() const
    {
        return init_problems_called_;
    }

private:
    bool launch_called_;
    bool init_problems_called_;
};

/**
 * @brief Test fixture for SequentialExecutionStrategy tests
 */
class SequentialExecutionStrategyTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a mock sequential instance
        auto mock = std::make_unique<MockBendersSequential>();
        mock_ptr_ = mock.get(); // Keep raw pointer for verification

        // Create the strategy with the mock
        strategy_ = std::make_unique<SequentialExecutionStrategy>(std::move(mock));
    }

    std::unique_ptr<SequentialExecutionStrategy> strategy_;
    MockBendersSequential* mock_ptr_; // Non-owning pointer for verification
};

/**
 * @brief Test that BendersName is correctly delegated
 */
TEST_F(SequentialExecutionStrategyTest, BendersName)
{
    EXPECT_EQ(strategy_->BendersName(), "MockSequential");
}

/**
 * @brief Test that execution_time returns 0.0 initially
 */
TEST_F(SequentialExecutionStrategyTest, ExecutionTime)
{
    // Initially should be 0.0
    EXPECT_DOUBLE_EQ(strategy_->execution_time(), 0.0);
}

/**
 * @brief Test that launch is correctly delegated
 */
TEST_F(SequentialExecutionStrategyTest, LaunchDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_launch_called());

    strategy_->launch();

    EXPECT_TRUE(mock_ptr_->was_launch_called());
}

/**
 * @brief Test that InitializeProblems is correctly delegated
 */
TEST_F(SequentialExecutionStrategyTest, InitializeProblemsDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_init_problems_called());

    strategy_->InitializeProblems();

    EXPECT_TRUE(mock_ptr_->was_init_problems_called());
}

/**
 * @brief Test that Run is correctly delegated (calls launch internally)
 */
TEST_F(SequentialExecutionStrategyTest, RunDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_launch_called());

    strategy_->Run();

    EXPECT_TRUE(mock_ptr_->was_launch_called());
}

/**
 * @brief Test null safety - strategy with null sequential pointer
 */
TEST(SequentialExecutionStrategyNullTest, NullSafety)
{
    SequentialExecutionStrategy strategy(nullptr);

    // Should not crash with null pointer
    EXPECT_NO_THROW(strategy.launch());
    EXPECT_NO_THROW(strategy.InitializeProblems());
    EXPECT_NO_THROW(strategy.Run());

    // Should return safe defaults
    EXPECT_EQ(strategy.BendersName(), "SequentialExecutionStrategy");
    EXPECT_DOUBLE_EQ(strategy.execution_time(), 0.0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
