#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/strategy/BendersCore.h"
#include "antares-xpansion/benders/strategy/IExecutionStrategy.h"
#include "antares-xpansion/benders/strategy/IBatchingStrategy.h"
#include "antares-xpansion/benders/strategy/IOuterLoopStrategy.h"

/**
 * @brief Mock ExecutionStrategy for testing
 */
class MockExecutionStrategy : public IExecutionStrategy
{
public:
    MockExecutionStrategy()
        : launch_called_(false),
          init_problems_called_(false),
          run_called_(false),
          name_("MockExecution"),
          exec_time_(1.5)
    {
    }

    void launch() override { launch_called_ = true; }
    void InitializeProblems() override { init_problems_called_ = true; }
    void Run() override { run_called_ = true; }
    
    [[nodiscard]] std::string BendersName() const override { return name_; }
    [[nodiscard]] double execution_time() const override { return exec_time_; }

    bool WasLaunchCalled() const { return launch_called_; }
    bool WasInitProblemsCalled() const { return init_problems_called_; }
    bool WasRunCalled() const { return run_called_; }
    
    void SetName(const std::string& name) { name_ = name; }
    void SetExecutionTime(double time) { exec_time_ = time; }

private:
    bool launch_called_;
    bool init_problems_called_;
    bool run_called_;
    std::string name_;
    double exec_time_;
};

/**
 * @brief Mock BatchingStrategy for testing
 */
class MockBatchingStrategy : public IBatchingStrategy
{
public:
    MockBatchingStrategy()
        : init_problems_called_(false),
          update_criterion_called_(false),
          should_stop_(false)
    {
    }

    void InitializeProblems() override { init_problems_called_ = true; }
    void UpdateStoppingCriterion() override { update_criterion_called_ = true; }
    [[nodiscard]] bool ShouldRelaxationStop() const override { return should_stop_; }

    bool WasInitProblemsCalled() const { return init_problems_called_; }
    bool WasUpdateCriterionCalled() const { return update_criterion_called_; }
    
    void SetShouldStop(bool value) { should_stop_ = value; }

private:
    bool init_problems_called_;
    bool update_criterion_called_;
    bool should_stop_;
};

/**
 * @brief Mock OuterLoopStrategy for testing
 */
class MockOuterLoopStrategy : public IOuterLoopStrategy
{
public:
    MockOuterLoopStrategy()
        : run_called_(false),
          run_attached_called_(false),
          update_master_result_(false),
          print_log_called_(false),
          init_data_called_(false),
          exception_raised_(false),
          lambda_min_(0.0),
          lambda_max_(0.0),
          check_feasibility_called_(false),
          bilevel_checks_called_(false)
    {
    }

    void Run() override { run_called_ = true; }
    void RunAttachedAlgo() override { run_attached_called_ = true; }
    bool UpdateMaster() override { return update_master_result_; }
    void PrintLog() override { print_log_called_ = true; }
    void init_data() override { init_data_called_ = true; }
    bool isExceptionRaised() override { return exception_raised_; }
    [[nodiscard]] double OuterLoopLambdaMin() const override { return lambda_min_; }
    [[nodiscard]] double OuterLoopLambdaMax() const override { return lambda_max_; }
    void OuterLoopCheckFeasibility() override { check_feasibility_called_ = true; }
    void OuterLoopBilevelChecks() override { bilevel_checks_called_ = true; }

    bool WasRunCalled() const { return run_called_; }
    bool WasRunAttachedCalled() const { return run_attached_called_; }
    bool WasInitDataCalled() const { return init_data_called_; }
    
    void SetUpdateMasterResult(bool value) { update_master_result_ = value; }

private:
    bool run_called_;
    bool run_attached_called_;
    bool update_master_result_;
    bool print_log_called_;
    bool init_data_called_;
    bool exception_raised_;
    double lambda_min_;
    double lambda_max_;
    bool check_feasibility_called_;
    bool bilevel_checks_called_;
};

/**
 * @brief Test fixture for BendersCore tests
 */
class BendersCoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mock strategies
        auto exec = std::make_unique<MockExecutionStrategy>();
        auto batch = std::make_unique<MockBatchingStrategy>();
        auto outer = std::make_unique<MockOuterLoopStrategy>();
        
        // Keep raw pointers for verification
        exec_ptr_ = exec.get();
        batch_ptr_ = batch.get();
        outer_ptr_ = outer.get();
        
        // Create BendersCore with mock strategies
        core_ = std::make_unique<BendersCore>(
            std::move(exec),
            std::move(batch),
            std::move(outer)
        );
    }

    std::unique_ptr<BendersCore> core_;
    MockExecutionStrategy* exec_ptr_;
    MockBatchingStrategy* batch_ptr_;
    MockOuterLoopStrategy* outer_ptr_;
};

/**
 * @brief Test BendersName reflects execution strategy
 */
TEST_F(BendersCoreTest, BendersName)
{
    exec_ptr_->SetName("TestExecution");
    EXPECT_EQ(core_->BendersName(), "BendersCore(TestExecution)");
}

/**
 * @brief Test execution_time delegates to execution strategy
 */
TEST_F(BendersCoreTest, ExecutionTime)
{
    exec_ptr_->SetExecutionTime(42.5);
    EXPECT_DOUBLE_EQ(core_->execution_time(), 42.5);
}

/**
 * @brief Test InitializeProblems calls both batching and execution
 */
TEST_F(BendersCoreTest, InitializeProblems)
{
    EXPECT_FALSE(batch_ptr_->WasInitProblemsCalled());
    EXPECT_FALSE(exec_ptr_->WasInitProblemsCalled());
    
    core_->InitializeProblems();
    
    EXPECT_TRUE(batch_ptr_->WasInitProblemsCalled());
    EXPECT_TRUE(exec_ptr_->WasInitProblemsCalled());
}

/**
 * @brief Test launch orchestrates all strategies in correct order
 */
TEST_F(BendersCoreTest, LaunchOrchestration)
{
    EXPECT_FALSE(outer_ptr_->WasInitDataCalled());
    EXPECT_FALSE(batch_ptr_->WasInitProblemsCalled());
    EXPECT_FALSE(exec_ptr_->WasInitProblemsCalled());
    EXPECT_FALSE(outer_ptr_->WasRunCalled());
    EXPECT_FALSE(batch_ptr_->WasUpdateCriterionCalled());
    
    core_->launch();
    
    // Verify all initialization steps occurred
    EXPECT_TRUE(outer_ptr_->WasInitDataCalled());
    EXPECT_TRUE(batch_ptr_->WasInitProblemsCalled());
    EXPECT_TRUE(exec_ptr_->WasInitProblemsCalled());
    
    // Verify outer loop runs (not execution directly, since outer loop is active)
    EXPECT_TRUE(outer_ptr_->WasRunCalled());
    EXPECT_FALSE(exec_ptr_->WasRunCalled());
    
    // Verify batching update called
    EXPECT_TRUE(batch_ptr_->WasUpdateCriterionCalled());
}

/**
 * @brief Test launch without outer loop runs execution directly
 */
TEST(BendersCoreNoOuterLoopTest, LaunchWithoutOuterLoop)
{
    auto exec = std::make_unique<MockExecutionStrategy>();
    auto batch = std::make_unique<MockBatchingStrategy>();
    
    auto exec_ptr = exec.get();
    
    // Create core without outer loop
    auto core = std::make_unique<BendersCore>(
        std::move(exec),
        std::move(batch),
        nullptr  // No outer loop
    );
    
    EXPECT_FALSE(exec_ptr->WasRunCalled());
    
    core->launch();
    
    // Without outer loop, execution should run directly
    EXPECT_TRUE(exec_ptr->WasRunCalled());
}

/**
 * @brief Test null safety - core with null strategies
 */
TEST(BendersCoreNullTest, NullSafety)
{
    // Create core with null strategies
    BendersCore core(nullptr, nullptr, nullptr);
    
    // Should not crash
    EXPECT_NO_THROW(core.launch());
    EXPECT_NO_THROW(core.InitializeProblems());
    
    // Should return safe defaults
    EXPECT_EQ(core.BendersName(), "BendersCore(NoExecution)");
    EXPECT_DOUBLE_EQ(core.execution_time(), 0.0);
}

/**
 * @brief Test null safety - core with only execution strategy
 */
TEST(BendersCorePartialNullTest, OnlyExecutionStrategy)
{
    auto exec = std::make_unique<MockExecutionStrategy>();
    auto exec_ptr = exec.get();
    
    BendersCore core(std::move(exec), nullptr, nullptr);
    
    EXPECT_NO_THROW(core.launch());
    
    // Execution should run since there's no outer loop
    EXPECT_TRUE(exec_ptr->WasRunCalled());
}

/**
 * @brief Test different strategy combinations
 */
class BendersCoreIntegrationTest : public ::testing::Test
{
protected:
    std::unique_ptr<BendersCore> CreateCore(
        bool with_execution,
        bool with_batching,
        bool with_outer_loop)
    {
        std::unique_ptr<IExecutionStrategy> exec = with_execution 
            ? std::make_unique<MockExecutionStrategy>() 
            : nullptr;
        
        std::unique_ptr<IBatchingStrategy> batch = with_batching
            ? std::make_unique<MockBatchingStrategy>()
            : nullptr;
        
        std::unique_ptr<IOuterLoopStrategy> outer = with_outer_loop
            ? std::make_unique<MockOuterLoopStrategy>()
            : nullptr;
        
        return std::make_unique<BendersCore>(
            std::move(exec),
            std::move(batch),
            std::move(outer)
        );
    }
};

/**
 * @brief Test combination: Execution + NoBatch + NoOuterLoop
 */
TEST_F(BendersCoreIntegrationTest, ExecutionOnly)
{
    auto core = CreateCore(true, false, false);
    EXPECT_NO_THROW(core->launch());
    EXPECT_NE(core->BendersName(), "");
}

/**
 * @brief Test combination: Execution + Batch + NoOuterLoop
 */
TEST_F(BendersCoreIntegrationTest, ExecutionWithBatching)
{
    auto core = CreateCore(true, true, false);
    EXPECT_NO_THROW(core->launch());
    EXPECT_NE(core->BendersName(), "");
}

/**
 * @brief Test combination: Execution + NoBatch + OuterLoop
 */
TEST_F(BendersCoreIntegrationTest, ExecutionWithOuterLoop)
{
    auto core = CreateCore(true, false, true);
    EXPECT_NO_THROW(core->launch());
    EXPECT_NE(core->BendersName(), "");
}

/**
 * @brief Test combination: Execution + Batch + OuterLoop (full configuration)
 */
TEST_F(BendersCoreIntegrationTest, FullConfiguration)
{
    auto core = CreateCore(true, true, true);
    EXPECT_NO_THROW(core->launch());
    EXPECT_NE(core->BendersName(), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
