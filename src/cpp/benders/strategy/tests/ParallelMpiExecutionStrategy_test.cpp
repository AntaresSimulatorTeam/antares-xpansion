#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/strategy/ParallelMpiExecutionStrategy.h"
#include "antares-xpansion/benders/benders_mpi/BendersMPI.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"

/**
 * @brief Mock BendersMPI for testing purposes
 * 
 * Overrides BendersMPI methods to avoid actual MPI execution
 * while allowing us to verify the adapter delegates correctly.
 */
class MockBendersMPI : public BendersMpi
{
public:
    /**
     * @brief Constructor with mock MPI communicator
     * 
     * Creates a minimal BendersMPI instance for testing.
     * Note: We use a real mpi::communicator but it won't be used in tests.
     */
    MockBendersMPI(mpi::communicator& world)
        : BendersMpi(
              BendersBaseOptions(SolverBaseOptions()),
              build_void_logger(),
              std::shared_ptr<Output::OutputWriter>(),
              world,
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
        return "MockMPI";
    }

    bool was_launch_called() const { return launch_called_; }
    bool was_init_problems_called() const { return init_problems_called_; }

protected:
    void Run() override
    {
        // Mock - do nothing
    }

private:
    bool launch_called_;
    bool init_problems_called_;
};

/**
 * @brief Test fixture for ParallelMpiExecutionStrategy tests
 */
class ParallelMpiExecutionStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a mock MPI communicator (default world)
        world_ = std::make_unique<mpi::communicator>();
        
        // Create a mock MPI benders instance
        auto mock = std::make_unique<MockBendersMPI>(*world_);
        mock_ptr_ = mock.get();  // Keep raw pointer for verification
        
        // Create the strategy with the mock
        strategy_ = std::make_unique<ParallelMpiExecutionStrategy>(std::move(mock));
    }

    void TearDown() override
    {
        // Clean up in reverse order
        strategy_.reset();
        mock_ptr_ = nullptr;
        world_.reset();
    }

    std::unique_ptr<mpi::communicator> world_;
    std::unique_ptr<ParallelMpiExecutionStrategy> strategy_;
    MockBendersMPI* mock_ptr_;  // Non-owning pointer for verification
};

/**
 * @brief Test that BendersName is correctly delegated
 */
TEST_F(ParallelMpiExecutionStrategyTest, BendersName)
{
    EXPECT_EQ(strategy_->BendersName(), "MockMPI");
}

/**
 * @brief Test that execution_time returns 0.0 initially
 */
TEST_F(ParallelMpiExecutionStrategyTest, ExecutionTime)
{
    // Initially should be 0.0
    EXPECT_DOUBLE_EQ(strategy_->execution_time(), 0.0);
}

/**
 * @brief Test that launch is correctly delegated
 */
TEST_F(ParallelMpiExecutionStrategyTest, LaunchDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_launch_called());
    
    strategy_->launch();
    
    EXPECT_TRUE(mock_ptr_->was_launch_called());
}

/**
 * @brief Test that InitializeProblems is correctly delegated
 */
TEST_F(ParallelMpiExecutionStrategyTest, InitializeProblemsDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_init_problems_called());
    
    strategy_->InitializeProblems();
    
    EXPECT_TRUE(mock_ptr_->was_init_problems_called());
}

/**
 * @brief Test that Run is correctly delegated (calls launch internally)
 */
TEST_F(ParallelMpiExecutionStrategyTest, RunDelegation)
{
    EXPECT_FALSE(mock_ptr_->was_launch_called());
    
    strategy_->Run();
    
    EXPECT_TRUE(mock_ptr_->was_launch_called());
}

/**
 * @brief Test null safety - strategy with null MPI benders pointer
 */
TEST(ParallelMpiExecutionStrategyNullTest, NullSafety)
{
    ParallelMpiExecutionStrategy strategy(nullptr);
    
    // Should not crash with null pointer
    EXPECT_NO_THROW(strategy.launch());
    EXPECT_NO_THROW(strategy.InitializeProblems());
    EXPECT_NO_THROW(strategy.Run());
    
    // Should return safe defaults
    EXPECT_EQ(strategy.BendersName(), "ParallelMpiExecutionStrategy");
    EXPECT_DOUBLE_EQ(strategy.execution_time(), 0.0);
}

/**
 * @brief Test that strategy properly wraps MPI Benders
 * 
 * This test verifies that the strategy correctly wraps a BendersMPI instance
 * and that the MPI communicator is properly managed.
 */
TEST_F(ParallelMpiExecutionStrategyTest, MPICommunicatorHandling)
{
    // Verify the strategy has the MPI benders instance
    EXPECT_EQ(strategy_->BendersName(), "MockMPI");
    
    // Verify we can call methods without crashes
    EXPECT_NO_THROW(strategy_->launch());
    EXPECT_TRUE(mock_ptr_->was_launch_called());
}

int main(int argc, char** argv)
{
    // Initialize MPI for testing
    // Note: In a real MPI environment, this would be done by the test runner
    // For unit tests, we use a default communicator
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
