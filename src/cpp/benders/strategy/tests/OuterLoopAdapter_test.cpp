#include <gtest/gtest.h>
#include <memory>

#include "antares-xpansion/benders/strategy/OuterLoopAdapter.h"
#include "antares-xpansion/benders/outer_loop/OuterLoop.h"

/**
 * @brief Mock concrete OuterLoop implementation for testing
 */
class MockOuterLoop : public Outerloop::OuterLoop
{
public:
    MockOuterLoop()
        : run_called_(false),
          run_attached_called_(false),
          update_master_result_(false),
          print_log_called_(false),
          init_data_called_(false),
          exception_raised_(false),
          lambda_min_(1.0),
          lambda_max_(10.0),
          check_feasibility_called_(false),
          bilevel_checks_called_(false)
    {
    }

    void RunAttachedAlgo() override
    {
        run_attached_called_ = true;
    }

    bool UpdateMaster() override
    {
        return update_master_result_;
    }

    void PrintLog() override
    {
        print_log_called_ = true;
    }

    void init_data() override
    {
        init_data_called_ = true;
    }

    bool isExceptionRaised() override
    {
        return exception_raised_;
    }

    double OuterLoopLambdaMin() const override
    {
        return lambda_min_;
    }

    double OuterLoopLambdaMax() const override
    {
        return lambda_max_;
    }

    void OuterLoopCheckFeasibility() override
    {
        check_feasibility_called_ = true;
    }

    void OuterLoopBilevelChecks() override
    {
        bilevel_checks_called_ = true;
    }

    // Setters for test control
    void SetUpdateMasterResult(bool value) { update_master_result_ = value; }
    void SetExceptionRaised(bool value) { exception_raised_ = value; }
    void SetLambdaMin(double value) { lambda_min_ = value; }
    void SetLambdaMax(double value) { lambda_max_ = value; }

    // Getters for verification
    bool WasRunAttachedCalled() const { return run_attached_called_; }
    bool WasPrintLogCalled() const { return print_log_called_; }
    bool WasInitDataCalled() const { return init_data_called_; }
    bool WasCheckFeasibilityCalled() const { return check_feasibility_called_; }
    bool WasBilevelChecksCalled() const { return bilevel_checks_called_; }

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
 * @brief Test fixture for OuterLoopAdapter tests
 */
class OuterLoopAdapterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto mock = std::make_unique<MockOuterLoop>();
        mock_ptr_ = mock.get();
        strategy_ = std::make_unique<OuterLoopAdapter>(std::move(mock));
    }

    std::unique_ptr<OuterLoopAdapter> strategy_;
    MockOuterLoop* mock_ptr_;
};

/**
 * @brief Test RunAttachedAlgo delegation
 */
TEST_F(OuterLoopAdapterTest, RunAttachedAlgoDelegation)
{
    EXPECT_FALSE(mock_ptr_->WasRunAttachedCalled());
    
    strategy_->RunAttachedAlgo();
    
    EXPECT_TRUE(mock_ptr_->WasRunAttachedCalled());
}

/**
 * @brief Test UpdateMaster delegation
 */
TEST_F(OuterLoopAdapterTest, UpdateMasterDelegation)
{
    // Initially false
    EXPECT_FALSE(strategy_->UpdateMaster());
    
    // Change mock behavior
    mock_ptr_->SetUpdateMasterResult(true);
    EXPECT_TRUE(strategy_->UpdateMaster());
    
    // Change back
    mock_ptr_->SetUpdateMasterResult(false);
    EXPECT_FALSE(strategy_->UpdateMaster());
}

/**
 * @brief Test PrintLog delegation
 */
TEST_F(OuterLoopAdapterTest, PrintLogDelegation)
{
    EXPECT_FALSE(mock_ptr_->WasPrintLogCalled());
    
    strategy_->PrintLog();
    
    EXPECT_TRUE(mock_ptr_->WasPrintLogCalled());
}

/**
 * @brief Test init_data delegation
 */
TEST_F(OuterLoopAdapterTest, InitDataDelegation)
{
    EXPECT_FALSE(mock_ptr_->WasInitDataCalled());
    
    strategy_->init_data();
    
    EXPECT_TRUE(mock_ptr_->WasInitDataCalled());
}

/**
 * @brief Test isExceptionRaised delegation
 */
TEST_F(OuterLoopAdapterTest, IsExceptionRaisedDelegation)
{
    EXPECT_FALSE(strategy_->isExceptionRaised());
    
    mock_ptr_->SetExceptionRaised(true);
    EXPECT_TRUE(strategy_->isExceptionRaised());
    
    mock_ptr_->SetExceptionRaised(false);
    EXPECT_FALSE(strategy_->isExceptionRaised());
}

/**
 * @brief Test lambda values delegation
 */
TEST_F(OuterLoopAdapterTest, LambdaValuesDelegation)
{
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMin(), 1.0);
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMax(), 10.0);
    
    mock_ptr_->SetLambdaMin(2.5);
    mock_ptr_->SetLambdaMax(20.0);
    
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMin(), 2.5);
    EXPECT_DOUBLE_EQ(strategy_->OuterLoopLambdaMax(), 20.0);
}

/**
 * @brief Test OuterLoopCheckFeasibility delegation
 */
TEST_F(OuterLoopAdapterTest, CheckFeasibilityDelegation)
{
    EXPECT_FALSE(mock_ptr_->WasCheckFeasibilityCalled());
    
    strategy_->OuterLoopCheckFeasibility();
    
    EXPECT_TRUE(mock_ptr_->WasCheckFeasibilityCalled());
}

/**
 * @brief Test OuterLoopBilevelChecks delegation
 */
TEST_F(OuterLoopAdapterTest, BilevelChecksDelegation)
{
    EXPECT_FALSE(mock_ptr_->WasBilevelChecksCalled());
    
    strategy_->OuterLoopBilevelChecks();
    
    EXPECT_TRUE(mock_ptr_->WasBilevelChecksCalled());
}

/**
 * @brief Test null safety
 */
TEST(OuterLoopAdapterNullTest, NullSafety)
{
    OuterLoopAdapter strategy(nullptr);
    
    // Should not crash with null pointer
    EXPECT_NO_THROW(strategy.Run());
    EXPECT_NO_THROW(strategy.RunAttachedAlgo());
    EXPECT_NO_THROW(strategy.PrintLog());
    EXPECT_NO_THROW(strategy.init_data());
    EXPECT_NO_THROW(strategy.OuterLoopCheckFeasibility());
    EXPECT_NO_THROW(strategy.OuterLoopBilevelChecks());
    
    // Should return safe defaults
    EXPECT_FALSE(strategy.UpdateMaster());
    EXPECT_FALSE(strategy.isExceptionRaised());
    EXPECT_DOUBLE_EQ(strategy.OuterLoopLambdaMin(), 0.0);
    EXPECT_DOUBLE_EQ(strategy.OuterLoopLambdaMax(), 0.0);
}

/**
 * @brief Test complete workflow
 */
TEST_F(OuterLoopAdapterTest, CompleteWorkflow)
{
    EXPECT_NO_THROW({
        strategy_->init_data();
        EXPECT_TRUE(mock_ptr_->WasInitDataCalled());
        
        strategy_->Run();
        strategy_->RunAttachedAlgo();
        EXPECT_TRUE(mock_ptr_->WasRunAttachedCalled());
        
        EXPECT_FALSE(strategy_->UpdateMaster());
        
        strategy_->OuterLoopCheckFeasibility();
        EXPECT_TRUE(mock_ptr_->WasCheckFeasibilityCalled());
        
        strategy_->OuterLoopBilevelChecks();
        EXPECT_TRUE(mock_ptr_->WasBilevelChecksCalled());
        
        strategy_->PrintLog();
        EXPECT_TRUE(mock_ptr_->WasPrintLogCalled());
        
        EXPECT_FALSE(strategy_->isExceptionRaised());
    });
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
