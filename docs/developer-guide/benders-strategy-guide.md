# Benders Strategy Pattern - Developer Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [Using the Strategy Pattern](#using-the-strategy-pattern)
3. [Adding New Strategies](#adding-new-strategies)
4. [Common Patterns](#common-patterns)
5. [Best Practices](#best-practices)
6. [Troubleshooting](#troubleshooting)

## Quick Start

### Basic Usage

The Strategy pattern is transparent to users. Use the factory as before:

```cpp
#include "antares-xpansion/benders/factories/BendersFactory.h"

// Create factory
BendersFactory factory(dependencies);

// Get configured Benders environment
auto env = factory.PrepareForExecution(outer_loop);

// env->benders is now IBendersCore* (not BendersBase*)
env->benders->launch();

// Get results
auto results = env->benders->GetBestIterationData();
```

**That's it!** The factory automatically:
- Selects Sequential or MPI based on world size
- Chooses batching strategy based on BENDERSMETHOD
- Configures outer-loop based on options

### What Changed?

**Before** (inheritance hierarchy):
```cpp
BendersBase* benders = factory.create();
benders->launch();
```

**Now** (strategy pattern):
```cpp
IBendersCore* benders = env->benders;  // BendersCore instance
benders->launch();
```

The interface is the same! `IBendersCore` provides all methods from `BendersBase`.

## Using the Strategy Pattern

### Understanding the Factory

The factory creates a `BendersCore` with three composed strategies:

```cpp
// Inside BendersFactory::ConfigureBenders()

// 1. Select execution strategy based on MPI world size
if (world_->size() == 1) {
    // Single process → Sequential
    auto sequential = std::make_unique<BendersSequential>(...);
    execution = std::make_unique<SequentialExecutionStrategy>(
        std::move(sequential)
    );
} else {
    // Multiple processes → MPI
    auto mpi = std::make_unique<BendersMpi>(...);
    execution = std::make_unique<ParallelMpiExecutionStrategy>(
        std::move(mpi)
    );
}

// 2. Select batching strategy based on method
if (method == BENDERS_BY_BATCH || method == BENDERS_BY_BATCH_OUTERLOOP) {
    batching = std::make_unique<ByBatchStrategy>(...);
} else {
    batching = std::make_unique<NoBatchingStrategy>();
}

// 3. Select outer-loop strategy based on method
if (method == BENDERS_OUTERLOOP || method == BENDERS_BY_BATCH_OUTERLOOP) {
    outer_loop = std::make_unique<OuterLoopAdapter>(...);
} else {
    outer_loop = std::make_unique<NoOuterLoopStrategy>();
}

// 4. Compose into BendersCore
return std::make_unique<BendersCore>(
    std::move(execution),
    std::move(batching),
    std::move(outer_loop)
);
```

### Accessing Strategy Components

If you need to access specific strategy behavior (rare), you can:

```cpp
// Get the configured benders instance
auto benders = env->benders;

// All operations go through IBendersCore interface
benders->InitializeProblems();
benders->set_input_map(coupling_map);
int row_idx = benders->MasterRowIndex("row_name");
benders->launch();
```

**Note**: You shouldn't need to access individual strategies directly. `BendersCore` handles all orchestration.

## Adding New Strategies

### Example: Adding a GPU Execution Strategy

Let's walk through adding a hypothetical GPU execution strategy.

#### Step 1: Create the Strategy Interface Implementation

```cpp
// src/cpp/benders/strategy/include/.../GpuExecutionStrategy.h
#pragma once
#include "antares-xpansion/benders/strategy/IExecutionStrategy.h"
#include "antares-xpansion/benders/benders_gpu/BendersGpu.h"
#include <memory>

class GpuExecutionStrategy : public IExecutionStrategy {
public:
    explicit GpuExecutionStrategy(std::unique_ptr<BendersGpu> gpu)
        : gpu_(std::move(gpu)) {}

    void Run() override {
        if (gpu_) gpu_->Run();
    }

    void InitializeProblems() override {
        if (gpu_) gpu_->InitializeProblems();
    }

    void set_input_map(const CouplingMap& map) override {
        if (gpu_) gpu_->set_input_map(map);
    }

    std::string BendersName() const override {
        return gpu_ ? gpu_->BendersName() : "GpuExecutionStrategy";
    }

    double execution_time() const override {
        return gpu_ ? gpu_->execution_time() : 0.0;
    }

    // Implement all other IExecutionStrategy methods...

private:
    std::unique_ptr<BendersGpu> gpu_;
};
```

#### Step 2: Create Unit Tests

```cpp
// src/cpp/benders/strategy/tests/GpuExecutionStrategy_test.cpp
#include "antares-xpansion/benders/strategy/GpuExecutionStrategy.h"
#include <gtest/gtest.h>

// Mock BendersGpu
class MockBendersGpu : public BendersGpu {
public:
    MOCK_METHOD(void, Run, (), (override));
    MOCK_METHOD(void, InitializeProblems, (), (override));
    // ... other mocks
};

TEST(GpuExecutionStrategyTest, DelegatesRun) {
    auto mock_gpu = std::make_unique<MockBendersGpu>();
    EXPECT_CALL(*mock_gpu, Run()).Times(1);
    
    GpuExecutionStrategy strategy(std::move(mock_gpu));
    strategy.Run();
}

// ... more tests
```

#### Step 3: Add to CMakeLists.txt

```cmake
# src/cpp/benders/strategy/CMakeLists.txt

# Add test executable
if(GTest_FOUND)
    add_executable(GpuExecutionStrategy_test
        tests/GpuExecutionStrategy_test.cpp
    )
    target_link_libraries(GpuExecutionStrategy_test
        PRIVATE
        GTest::GTest
        GTest::Main
        antaresXpansion::benders_strategy
    )
    add_test(NAME GpuExecutionStrategy_test 
             COMMAND GpuExecutionStrategy_test)
endif()
```

#### Step 4: Integrate with Factory

```cpp
// src/cpp/benders/factories/BendersFactory.cpp

std::unique_ptr<IExecutionStrategy> CreateExecutionStrategy(...) {
    // Check if GPU is available and requested
    if (HasGpuSupport() && options.use_gpu) {
        auto gpu = std::make_unique<BendersGpu>(...);
        return std::make_unique<GpuExecutionStrategy>(std::move(gpu));
    }
    
    // Existing logic for Sequential/MPI
    if (world_->size() == 1) {
        // Sequential...
    } else {
        // MPI...
    }
}
```

#### Step 5: Document the New Strategy

Update documentation to explain when and how GPU strategy is used.

### Template for New Strategy

```cpp
#pragma once
#include "antares-xpansion/benders/strategy/I[Type]Strategy.h"
#include <memory>

class MyNewStrategy : public I[Type]Strategy {
public:
    // Constructor - inject dependencies
    explicit MyNewStrategy(/* dependencies */);

    // Implement interface methods
    void SomeMethod() override {
        if (implementation_) {
            implementation_->SomeMethod();
        }
    }

    // Implement all required interface methods...

private:
    std::unique_ptr<Implementation> implementation_;
};
```

## Common Patterns

### Pattern 1: Null-Safe Delegation

All strategies use null-safe delegation:

```cpp
void Method() override {
    if (wrapped_instance_) {
        wrapped_instance_->Method();
    }
}
```

This allows strategies to work even if the wrapped instance is null (defensive programming).

### Pattern 2: Header-Only Strategies

Most strategies are header-only (no .cpp file):
- Simpler build
- Easier to inline
- No ABI issues
- Faster compilation

Only create a .cpp file if you have non-trivial implementation logic.

### Pattern 3: Adapter Pattern

Strategies often use the Adapter pattern to wrap existing classes:

```cpp
class StrategyAdapter : public IStrategy {
public:
    explicit StrategyAdapter(std::unique_ptr<LegacyClass> legacy)
        : legacy_(std::move(legacy)) {}
    
    void NewInterfaceMethod() override {
        legacy_->OldInterfaceMethod();  // Adapt
    }
    
private:
    std::unique_ptr<LegacyClass> legacy_;
};
```

### Pattern 4: No-Op Strategies

For concerns that don't apply, use no-op strategies:

```cpp
class NoOuterLoopStrategy : public IOuterLoopStrategy {
public:
    void Run(IBendersCore*) override { /* no-op */ }
    void init_data() override { /* no-op */ }
    bool UpdateMaster(WorkerMasterDataVect&) override { return false; }
    // ... all methods are no-ops
};
```

### Pattern 5: Strategy Composition in Tests

Test strategies in isolation using mocks:

```cpp
TEST(BendersCoreTest, UsesAllStrategies) {
    auto exec = std::make_unique<MockExecutionStrategy>();
    auto batch = std::make_unique<MockBatchingStrategy>();
    auto outer = std::make_unique<MockOuterLoopStrategy>();
    
    EXPECT_CALL(*exec, Run()).Times(1);
    EXPECT_CALL(*batch, InitializeProblems()).Times(1);
    EXPECT_CALL(*outer, init_data()).Times(1);
    
    BendersCore core(std::move(exec), std::move(batch), std::move(outer));
    core.launch();
}
```

## Best Practices

### 1. One Strategy, One Concern

Each strategy should handle **exactly one concern**:
- ✅ ExecutionStrategy handles execution (Sequential/MPI)
- ✅ BatchingStrategy handles batching
- ✅ OuterLoopStrategy handles outer-loop
- ❌ Don't mix concerns in one strategy

### 2. Keep Strategies Simple

Strategies should be thin wrappers:
- Delegate to existing implementations
- Minimal logic in the strategy itself
- Complex logic goes in the wrapped class

### 3. Test Each Strategy Independently

Don't test strategies through BendersCore:
```cpp
// ❌ Bad: Testing strategy through orchestrator
TEST(BendersCoreTest, ExecutionStrategyWorks) {
    BendersCore core(...);
    core.launch();  // Too much happening
}

// ✅ Good: Testing strategy directly
TEST(ExecutionStrategyTest, DelegatesRun) {
    ExecutionStrategy strategy(...);
    strategy.Run();  // Clear, focused
}
```

### 4. Use Dependency Injection

Always inject dependencies through constructor:
```cpp
// ✅ Good
class Strategy {
public:
    explicit Strategy(std::unique_ptr<Dependency> dep)
        : dep_(std::move(dep)) {}
private:
    std::unique_ptr<Dependency> dep_;
};

// ❌ Bad
class Strategy {
    Strategy() {
        dep_ = std::make_unique<Dependency>();  // Hard-coded
    }
};
```

### 5. Document Strategy Selection Logic

When adding factory logic, document when each strategy is used:
```cpp
// Sequential is used when running on a single process
if (world_->size() == 1) {
    return std::make_unique<SequentialExecutionStrategy>(...);
}
```

### 6. Maintain Interface Compatibility

When modifying interfaces:
- Add new optional methods with default implementations
- Don't remove methods (deprecate first)
- Update all implementations together

## Troubleshooting

### Problem: Strategy Not Selected

**Symptom**: Wrong strategy is being used

**Solution**: Check factory logic in `BendersFactory::ConfigureBenders()`:
```cpp
// Debug output
std::cout << "World size: " << world_->size() << std::endl;
std::cout << "Method: " << method << std::endl;
std::cout << "Selected execution: " << execution->BendersName() << std::endl;
```

### Problem: Null Pointer in Strategy

**Symptom**: Segmentation fault when calling strategy method

**Cause**: Wrapped instance is null

**Solution**: Check null safety in strategy:
```cpp
void Method() override {
    if (!wrapped_) {
        // Either log warning or throw exception
        throw std::runtime_error("Wrapped instance is null");
    }
    wrapped_->Method();
}
```

### Problem: Test Failures After Adding Strategy

**Symptom**: Existing tests fail after adding new strategy

**Cause**: Factory may be selecting wrong strategy in tests

**Solution**: 
1. Check test setup - is MPI world size configured correctly?
2. Mock the factory to return specific strategies
3. Use dependency injection in tests

```cpp
TEST(MyTest, WorksWithNewStrategy) {
    auto strategy = std::make_unique<MyNewStrategy>(...);
    BendersCore core(std::move(strategy), ...);
    // Test core with your strategy
}
```

### Problem: Compilation Error with Strategy

**Symptom**: "Method not found in interface"

**Cause**: Strategy doesn't implement all interface methods

**Solution**: Ensure all pure virtual methods are implemented:
```cpp
class IExecutionStrategy {
public:
    virtual void Run() = 0;           // Must implement
    virtual void InitializeProblems() = 0;  // Must implement
    // ... all pure virtual methods
};
```

### Problem: Strategy Test Segfaults

**Symptom**: Test crashes when testing strategy

**Cause**: Mock expectations not set up correctly

**Solution**: Set up all mock expectations:
```cpp
TEST(StrategyTest, Works) {
    auto mock = std::make_unique<MockImplementation>();
    
    // Set up ALL expectations before using strategy
    EXPECT_CALL(*mock, Method1()).Times(1);
    EXPECT_CALL(*mock, Method2()).WillOnce(Return(42));
    
    Strategy strategy(std::move(mock));
    strategy.DoSomething();
}
```

## Advanced Topics

### Custom Factory Methods

If you need custom strategy composition:

```cpp
std::unique_ptr<IBendersCore> CreateCustomBenders() {
    auto exec = std::make_unique<CustomExecutionStrategy>();
    auto batch = std::make_unique<NoBatchingStrategy>();
    auto outer = std::make_unique<NoOuterLoopStrategy>();
    
    return std::make_unique<BendersCore>(
        std::move(exec),
        std::move(batch),
        std::move(outer)
    );
}
```

### Runtime Strategy Swapping

Currently, strategies are set at construction. For runtime swapping:

1. Add setter methods to BendersCore (not recommended)
2. Or create a new BendersCore instance with different strategies

### Performance Considerations

- **Strategy overhead**: Minimal (virtual call overhead only)
- **Header-only**: Allows inlining of simple methods
- **Wrapping existing code**: No performance change vs. direct use

## Next Steps

- Read [Architecture Overview](../architecture/benders-strategy-overview.md)
- Study [Code Navigation Guide](code-navigation.md)
- Learn [Testing Practices](testing-strategy-pattern.md)
- Check [API Reference](../api/benders-strategy-api.md)
