# Testing Guide - Benders Strategy Pattern

## Table of Contents
1. [Testing Philosophy](#testing-philosophy)
2. [Test Structure](#test-structure)
3. [Testing Patterns](#testing-patterns)
4. [Running Tests](#running-tests)
5. [Adding New Tests](#adding-new-tests)
6. [Mocking Strategies](#mocking-strategies)

## Testing Philosophy

### Principles

1. **Test in Isolation**: Each strategy tested independently
2. **Mock Dependencies**: Use mocks for wrapped implementations
3. **Test Delegation**: Verify strategies delegate correctly
4. **Test Null Safety**: Ensure strategies handle null gracefully
5. **Test Workflows**: Verify end-to-end scenarios work

### Test Coverage

Current test suite (66 tests total):

| Component | Tests | Coverage |
|-----------|-------|----------|
| SequentialExecutionStrategy | 6 | Method delegation, null safety |
| ParallelMpiExecutionStrategy | 7 | Method delegation, MPI handling, null safety |
| NoBatchingStrategy | 5 | Passthrough behavior, lifecycle |
| ByBatchStrategy | 6 | Delegation, batching logic, null safety |
| NoOuterLoopStrategy | 11 | Passthrough behavior, all methods |
| OuterLoopAdapter | 10 | Delegation, outer-loop logic, null safety |
| BendersCore | 13 | Orchestration, delegation, combinations |
| BendersFactory | 8 | Method deduction, strategy selection |

## Test Structure

### Standard Test File Structure

```cpp
// MyStrategy_test.cpp

// 1. Includes
#include "antares-xpansion/benders/strategy/MyStrategy.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// 2. Mock Class (if needed)
class MockImplementation : public RealImplementation {
public:
    MOCK_METHOD(void, SomeMethod, (), (override));
    MOCK_METHOD(int, AnotherMethod, (const std::string&), (const, override));
    // ... mock all needed methods
};

// 3. Test Fixture (optional, for complex setup)
class MyStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    // Helper members
};

// 4. Test Cases
TEST(MyStrategyTest, DelegatesMethodCorrectly) {
    // Arrange
    auto mock = std::make_unique<MockImplementation>();
    EXPECT_CALL(*mock, SomeMethod()).Times(1);
    
    MyStrategy strategy(std::move(mock));
    
    // Act
    strategy.SomeMethod();
    
    // Assert
    // Expectations verified automatically
}

TEST(MyStrategyTest, HandlesNullGracefully) {
    // Arrange
    MyStrategy strategy(nullptr);  // Null implementation
    
    // Act & Assert (should not crash)
    EXPECT_NO_THROW(strategy.SomeMethod());
}

// 5. More test cases...
```

## Testing Patterns

### Pattern 1: Testing Delegation

**Purpose**: Verify strategy delegates to wrapped implementation

```cpp
TEST(SequentialExecutionStrategyTest, DelegatesLaunch) {
    // Create mock
    auto mock_sequential = std::make_unique<MockBendersSequential>();
    
    // Set expectation
    EXPECT_CALL(*mock_sequential, launch()).Times(1);
    
    // Create strategy with mock
    SequentialExecutionStrategy strategy(std::move(mock_sequential));
    
    // Call method
    strategy.launch();
    
    // Mock verifies launch() was called exactly once
}
```

### Pattern 2: Testing Return Values

**Purpose**: Verify strategy returns correct values from wrapped implementation

```cpp
TEST(SequentialExecutionStrategyTest, ReturnsBendersName) {
    // Create mock
    auto mock_sequential = std::make_unique<MockBendersSequential>();
    
    // Set expectation with return value
    EXPECT_CALL(*mock_sequential, BendersName())
        .WillOnce(::testing::Return("MockBenders"));
    
    // Create strategy
    SequentialExecutionStrategy strategy(std::move(mock_sequential));
    
    // Call and verify return value
    EXPECT_EQ(strategy.BendersName(), "MockBenders");
}
```

### Pattern 3: Testing Null Safety

**Purpose**: Ensure strategy doesn't crash with null wrapped instance

```cpp
TEST(SequentialExecutionStrategyTest, HandlesNullSafely) {
    // Create strategy with null
    SequentialExecutionStrategy strategy(nullptr);
    
    // These should not crash
    EXPECT_NO_THROW(strategy.launch());
    EXPECT_NO_THROW(strategy.InitializeProblems());
    
    // Methods that return values should return safe defaults
    EXPECT_EQ(strategy.BendersName(), "SequentialExecutionStrategy");
    EXPECT_EQ(strategy.execution_time(), 0.0);
}
```

### Pattern 4: Testing Passthrough Strategies

**Purpose**: Verify no-op strategies do nothing (and don't crash)

```cpp
TEST(NoBatchingStrategyTest, InitializeProblemsIsNoOp) {
    NoBatchingStrategy strategy;
    
    // Should not crash
    EXPECT_NO_THROW(strategy.InitializeProblems());
}

TEST(NoBatchingStrategyTest, ShouldRelaxationStopReturnsFalse) {
    NoBatchingStrategy strategy;
    
    // Always returns false (no batching logic)
    EXPECT_FALSE(strategy.ShouldRelaxationStop());
}
```

### Pattern 5: Testing Orchestration

**Purpose**: Verify BendersCore orchestrates strategies correctly

```cpp
TEST(BendersCoreTest, LaunchOrchestratesStrategies) {
    // Create mock strategies
    auto mock_exec = std::make_unique<MockExecutionStrategy>();
    auto mock_batch = std::make_unique<MockBatchingStrategy>();
    auto mock_outer = std::make_unique<MockOuterLoopStrategy>();
    
    // Set expectations in order
    {
        ::testing::InSequence seq;
        EXPECT_CALL(*mock_outer, init_data()).Times(1);
        EXPECT_CALL(*mock_batch, InitializeProblems()).Times(1);
        EXPECT_CALL(*mock_exec, InitializeProblems()).Times(1);
        EXPECT_CALL(*mock_outer, Run(::testing::_)).Times(1);
        EXPECT_CALL(*mock_batch, UpdateStoppingCriterion()).Times(1);
    }
    
    // Create orchestrator
    BendersCore core(
        std::move(mock_exec),
        std::move(mock_batch),
        std::move(mock_outer)
    );
    
    // Execute
    core.launch();
    
    // Expectations verified automatically
}
```

### Pattern 6: Testing Factory Logic

**Purpose**: Verify factory creates correct strategies

```cpp
TEST(BendersFactoryTest, DeducesBendersMethod) {
    // Test all BENDERSMETHOD deduction cases
    
    // No batching, no outer loop
    EXPECT_EQ(DeduceBendersMethod(100, 0, false), BENDERS);
    
    // No batching, with outer loop
    EXPECT_EQ(DeduceBendersMethod(100, 0, true), BENDERS_OUTERLOOP);
    
    // Batching (size < n-1), no outer loop
    EXPECT_EQ(DeduceBendersMethod(100, 5, false), BENDERS_BY_BATCH);
    
    // Batching, with outer loop
    EXPECT_EQ(DeduceBendersMethod(100, 5, true), BENDERS_BY_BATCH_OUTERLOOP);
}
```

### Pattern 7: Testing with Expectations

**Purpose**: Verify method parameters are passed correctly

```cpp
TEST(StrategyTest, PassesParametersCorrectly) {
    auto mock = std::make_unique<MockImplementation>();
    
    // Expect method called with specific parameter
    EXPECT_CALL(*mock, SetValue(42)).Times(1);
    
    Strategy strategy(std::move(mock));
    strategy.SetValue(42);
}
```

### Pattern 8: Testing Multiple Calls

**Purpose**: Verify strategies can be called multiple times

```cpp
TEST(StrategyTest, SupportsMultipleCalls) {
    auto mock = std::make_unique<MockImplementation>();
    
    // Expect method called multiple times
    EXPECT_CALL(*mock, Update()).Times(3);
    
    Strategy strategy(std::move(mock));
    
    // Call multiple times
    strategy.Update();
    strategy.Update();
    strategy.Update();
}
```

## Running Tests

### Run All Strategy Tests

```bash
cd build
ctest -R Strategy --output-on-failure
```

### Run Specific Strategy Tests

```bash
# Sequential execution strategy
ctest -R SequentialExecutionStrategy_test -V

# MPI execution strategy
ctest -R ParallelMpiExecutionStrategy_test -V

# Batching strategies
ctest -R NoBatchingStrategy_test -V
ctest -R ByBatchStrategy_test -V

# Outer-loop strategies
ctest -R NoOuterLoopStrategy_test -V
ctest -R OuterLoopAdapter_test -V

# Orchestrator
ctest -R BendersCore_test -V

# Factory
ctest -R BendersFactory_test -V
```

### Run Tests with Verbose Output

```bash
# Detailed output for debugging
ctest -R MyStrategy_test -VV

# Show test output even on success
ctest -R MyStrategy_test --output-on-failure
```

### Run Tests in Parallel

```bash
# Run tests in parallel (faster)
ctest -j 4
```

### Run with Valgrind (Memory Checks)

```bash
# Check for memory leaks
ctest -R MyStrategy_test -D ExperimentalMemCheck
```

## Adding New Tests

### Step 1: Create Test File

```bash
cd src/cpp/benders/strategy/tests/
touch MyNewStrategy_test.cpp
```

### Step 2: Write Test File

```cpp
#include "antares-xpansion/benders/strategy/MyNewStrategy.h"
#include <gtest/gtest.h>

// Mock if needed
class MockImplementation : public RealImplementation {
public:
    MOCK_METHOD(void, DoSomething, (), (override));
};

// Test cases
TEST(MyNewStrategyTest, BasicFunctionality) {
    auto mock = std::make_unique<MockImplementation>();
    EXPECT_CALL(*mock, DoSomething()).Times(1);
    
    MyNewStrategy strategy(std::move(mock));
    strategy.DoSomething();
}

TEST(MyNewStrategyTest, NullSafety) {
    MyNewStrategy strategy(nullptr);
    EXPECT_NO_THROW(strategy.DoSomething());
}
```

### Step 3: Add to CMakeLists.txt

```cmake
# src/cpp/benders/strategy/CMakeLists.txt

if(GTest_FOUND)
    add_executable(MyNewStrategy_test
        tests/MyNewStrategy_test.cpp
    )
    
    target_link_libraries(MyNewStrategy_test
        PRIVATE
        GTest::GTest
        GTest::Main
        antaresXpansion::benders_strategy
    )
    
    add_test(NAME MyNewStrategy_test 
             COMMAND MyNewStrategy_test)
endif()
```

### Step 4: Build and Run

```bash
cd build
cmake ..
make MyNewStrategy_test
ctest -R MyNewStrategy_test -V
```

## Mocking Strategies

### Creating Mock Strategies

For testing components that use strategies (e.g., BendersCore):

```cpp
// Create mock strategy interfaces
class MockExecutionStrategy : public IExecutionStrategy {
public:
    MOCK_METHOD(void, Run, (), (override));
    MOCK_METHOD(void, InitializeProblems, (), (override));
    MOCK_METHOD(void, set_input_map, (const CouplingMap&), (override));
    MOCK_METHOD(std::string, BendersName, (), (const, override));
    MOCK_METHOD(double, execution_time, (), (const, override));
    // ... mock all interface methods
};

class MockBatchingStrategy : public IBatchingStrategy {
public:
    MOCK_METHOD(void, InitializeProblems, (), (override));
    MOCK_METHOD(void, UpdateStoppingCriterion, (), (override));
    MOCK_METHOD(bool, ShouldRelaxationStop, (), (const, override));
};

class MockOuterLoopStrategy : public IOuterLoopStrategy {
public:
    MOCK_METHOD(void, Run, (IBendersCore*), (override));
    MOCK_METHOD(void, init_data, (), (override));
    MOCK_METHOD(bool, UpdateMaster, (WorkerMasterDataVect&), (override));
    // ... mock all interface methods
};
```

### Using Mock Strategies

```cpp
TEST(BendersCoreTest, WorksWithMocks) {
    // Create mocks
    auto mock_exec = std::make_unique<MockExecutionStrategy>();
    auto mock_batch = std::make_unique<MockBatchingStrategy>();
    auto mock_outer = std::make_unique<MockOuterLoopStrategy>();
    
    // Set expectations
    EXPECT_CALL(*mock_exec, Run()).Times(1);
    EXPECT_CALL(*mock_batch, InitializeProblems()).Times(1);
    EXPECT_CALL(*mock_outer, init_data()).Times(1);
    
    // Create BendersCore with mocks
    BendersCore core(
        std::move(mock_exec),
        std::move(mock_batch),
        std::move(mock_outer)
    );
    
    // Test
    core.launch();
}
```

## Common Testing Pitfalls

### Pitfall 1: Forgetting to Set Expectations

```cpp
// ❌ Bad: No expectations set
TEST(StrategyTest, Calls Method) {
    auto mock = std::make_unique<MockImpl>();
    Strategy strategy(std::move(mock));
    strategy.DoSomething();  // Might pass even if not implemented!
}

// ✅ Good: Expectations set
TEST(StrategyTest, CallsMethod) {
    auto mock = std::make_unique<MockImpl>();
    EXPECT_CALL(*mock, DoSomething()).Times(1);  // Verify it's called
    Strategy strategy(std::move(mock));
    strategy.DoSomething();
}
```

### Pitfall 2: Using Mock After Move

```cpp
// ❌ Bad: Using mock after moving
auto mock = std::make_unique<MockImpl>();
EXPECT_CALL(*mock, DoSomething()).Times(1);
Strategy strategy(std::move(mock));
mock->SomeOtherMethod();  // ERROR: mock is nullptr!

// ✅ Good: Set expectations before move
auto mock = std::make_unique<MockImpl>();
auto* mock_ptr = mock.get();  // Save pointer if needed
EXPECT_CALL(*mock, DoSomething()).Times(1);
Strategy strategy(std::move(mock));
// Use mock_ptr if you need to verify state (carefully!)
```

### Pitfall 3: Not Testing Null Safety

```cpp
// ❌ Bad: Assuming non-null
TEST(StrategyTest, Works) {
    auto impl = std::make_unique<RealImpl>();
    Strategy strategy(std::move(impl));
    strategy.DoSomething();  // What if impl is null?
}

// ✅ Good: Explicitly test null case
TEST(StrategyTest, HandlesNullSafely) {
    Strategy strategy(nullptr);
    EXPECT_NO_THROW(strategy.DoSomething());
}
```

## Test Maintenance

### When to Update Tests

- **Interface changes**: Update all strategy tests
- **New methods**: Add tests for new methods
- **Bug fixes**: Add regression test
- **Performance changes**: Update performance expectations

### Test Naming Conventions

```cpp
// Pattern: ClassName + Test + MethodName + Behavior
TEST(BendersCoreTest, LaunchCallsAllStrategies)
TEST(SequentialExecutionStrategyTest, DelegatesRun)
TEST(NoBatchingStrategyTest, ShouldRelaxationStopReturnsFalse)
```

### Organizing Tests

Group related tests:
```cpp
// Delegation tests
TEST(StrategyTest, DelegatesMethod1) { ... }
TEST(StrategyTest, DelegatesMethod2) { ... }

// Null safety tests
TEST(StrategyTest, HandlesNullInMethod1) { ... }
TEST(StrategyTest, HandlesNullInMethod2) { ... }

// Integration tests
TEST(StrategyTest, CompleteWorkflow) { ... }
```

## Next Steps

- Read [Developer Guide](benders-strategy-guide.md) for usage patterns
- See [Code Navigation](code-navigation.md) to find test files
- Check [Architecture Overview](../architecture/benders-strategy-overview.md) for design
- Review actual test files in `src/cpp/benders/strategy/tests/`
