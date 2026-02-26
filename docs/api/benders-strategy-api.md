# Benders Strategy Pattern - API Reference

**Overview**: See [Architecture Overview](../architecture/benders-strategy-overview.md) for high-level design and diagrams.

## Table of Contents
1. [Interfaces](#interfaces)
2. [Concrete Strategies](#concrete-strategies)
3. [Orchestrator](#orchestrator)
4. [Factory](#factory)
5. [Code Examples](#code-examples)

## Interfaces

### IBendersCore

**Purpose**: Main interface for Benders decomposition operations

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/IBendersCore.h`

**Methods**:

```cpp
class IBendersCore {
public:
    virtual ~IBendersCore() = default;
    
    // Execution control
    virtual void launch() = 0;
    virtual void InitializeProblems() = 0;
    virtual void free() = 0;
    virtual void DoFreeProblems(bool free_problems) = 0;
    
    // Configuration
    virtual void set_input_map(const CouplingMap& coupling_map) = 0;
    
    // Master problem access
    virtual int MasterRowIndex(const std::string& row_name) const = 0;
    virtual void MasterChangeRhs(int id_row, double val) const = 0;
    
    // Results access
    virtual LogData GetBestIterationData() const = 0;
    virtual WorkerMasterDataVect AllCuts() const = 0;
    
    // Output
    virtual void SaveOuterLoopSolutionInOutputFile() const = 0;
    
    // Metadata
    virtual std::string BendersName() const = 0;
    virtual double execution_time() const = 0;
};
```

**Usage**:
```cpp
IBendersCore* benders = /* from factory */;
benders->set_input_map(coupling_map);
benders->InitializeProblems();
benders->launch();
auto results = benders->GetBestIterationData();
```

---

### IExecutionStrategy

**Purpose**: Controls how subproblems are executed (Sequential vs. MPI)

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/IExecutionStrategy.h`

**Methods**:

```cpp
class IExecutionStrategy {
public:
    virtual ~IExecutionStrategy() = default;
    
    // Execution
    virtual void Run() = 0;
    virtual void InitializeProblems() = 0;
    
    // Configuration
    virtual void set_input_map(const CouplingMap& coupling_map) = 0;
    
    // Master problem access
    virtual int MasterRowIndex(const std::string& row_name) const = 0;
    virtual void MasterChangeRhs(int id_row, double val) const = 0;
    
    // Results
    virtual LogData GetBestIterationData() const = 0;
    virtual WorkerMasterDataVect AllCuts() const = 0;
    
    // Resource management
    virtual void free() = 0;
    virtual void DoFreeProblems(bool free_problems) = 0;
    
    // Metadata
    virtual std::string BendersName() const = 0;
    virtual double execution_time() const = 0;
};
```

**Implementations**:
- `SequentialExecutionStrategy` - Single-process execution
- `ParallelMpiExecutionStrategy` - Multi-process MPI execution

---

### IBatchingStrategy

**Purpose**: Controls problem batching behavior

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/IBatchingStrategy.h`

**Methods**:

```cpp
class IBatchingStrategy {
public:
    virtual ~IBatchingStrategy() = default;
    
    // Batching control
    virtual void InitializeProblems() = 0;
    virtual void UpdateStoppingCriterion() = 0;
    virtual bool ShouldRelaxationStop() const = 0;
};
```

**Implementations**:
- `NoBatchingStrategy` - No batching (passthrough)
- `ByBatchStrategy` - Process problems in batches

---

### IOuterLoopStrategy

**Purpose**: Controls outer-loop optimization

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/IOuterLoopStrategy.h`

**Methods**:

```cpp
class IOuterLoopStrategy {
public:
    virtual ~IOuterLoopStrategy() = default;
    
    // Outer-loop control
    virtual void Run(IBendersCore* benders) = 0;
    virtual void RunAttachedAlgo(unsigned int it) = 0;
    virtual void init_data() = 0;
    
    // Master problem updates
    virtual bool UpdateMaster(WorkerMasterDataVect& all_cuts) = 0;
    
    // Logging and output
    virtual void PrintLog() = 0;
    
    // Lambda management
    virtual double GetLambda(int id) const = 0;
    virtual void SetLambda(int id, double lambda) = 0;
    virtual Point GetLambdaVector() const = 0;
    
    // Status
    virtual bool isExceptionRaised() const = 0;
    
    // Feasibility checks
    virtual void CheckFeasibility() = 0;
    virtual void BilevelChecks() = 0;
};
```

**Implementations**:
- `NoOuterLoopStrategy` - No outer-loop (passthrough)
- `OuterLoopAdapter` - Outer-loop optimization enabled

---

## Concrete Strategies

### SequentialExecutionStrategy

**Purpose**: Wraps BendersSequential for single-process execution

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/SequentialExecutionStrategy.h`

**Constructor**:
```cpp
explicit SequentialExecutionStrategy(
    std::unique_ptr<BendersSequential> sequential
);
```

**Example**:
```cpp
auto sequential = std::make_unique<BendersSequential>(/* params */);
auto strategy = std::make_unique<SequentialExecutionStrategy>(
    std::move(sequential)
);
strategy->Run();
```

**Null Safety**: All methods check if wrapped instance is null before delegating.

---

### ParallelMpiExecutionStrategy

**Purpose**: Wraps BendersMPI for multi-process execution

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/ParallelMpiExecutionStrategy.h`

**Constructor**:
```cpp
explicit ParallelMpiExecutionStrategy(
    std::unique_ptr<BendersMpi> mpi
);
```

**Example**:
```cpp
auto mpi = std::make_unique<BendersMpi>(/* params */);
auto strategy = std::make_unique<ParallelMpiExecutionStrategy>(
    std::move(mpi)
);
strategy->Run();
```

**Null Safety**: All methods check if wrapped instance is null before delegating.

---

### NoBatchingStrategy

**Purpose**: Passthrough strategy when no batching is needed

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/NoBatchingStrategy.h`

**Constructor**:
```cpp
NoBatchingStrategy() = default;  // No parameters needed
```

**Example**:
```cpp
auto strategy = std::make_unique<NoBatchingStrategy>();
strategy->InitializeProblems();  // No-op
bool should_stop = strategy->ShouldRelaxationStop();  // Always false
```

**Behavior**: All methods are no-ops or return safe defaults.

---

### ByBatchStrategy

**Purpose**: Wraps BendersByBatch for batch processing

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/ByBatchStrategy.h`

**Constructor**:
```cpp
explicit ByBatchStrategy(
    std::unique_ptr<BendersByBatch> by_batch
);
```

**Example**:
```cpp
auto by_batch = std::make_unique<BendersByBatch>(/* params */);
auto strategy = std::make_unique<ByBatchStrategy>(
    std::move(by_batch)
);
strategy->InitializeProblems();
```

**Null Safety**: All methods check if wrapped instance is null before delegating.

---

### NoOuterLoopStrategy

**Purpose**: Passthrough strategy when no outer-loop is needed

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/NoOuterLoopStrategy.h`

**Constructor**:
```cpp
NoOuterLoopStrategy() = default;  // No parameters needed
```

**Example**:
```cpp
auto strategy = std::make_unique<NoOuterLoopStrategy>();
strategy->Run(benders);  // No-op
strategy->init_data();   // No-op
```

**Behavior**: All methods are no-ops or return safe defaults.

---

### OuterLoopAdapter

**Purpose**: Wraps Outerloop::OuterLoop for outer-loop optimization

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/OuterLoopAdapter.h`

**Constructor**:
```cpp
explicit OuterLoopAdapter(
    std::unique_ptr<Outerloop::OuterLoop> outer_loop
);
```

**Example**:
```cpp
auto outer_loop = std::make_unique<Outerloop::OuterLoop>(/* params */);
auto strategy = std::make_unique<OuterLoopAdapter>(
    std::move(outer_loop)
);
strategy->Run(benders);
```

**Null Safety**: All methods check if wrapped instance is null before delegating.

---

## Orchestrator

### BendersCore

**Purpose**: Orchestrates execution, batching, and outer-loop strategies

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/BendersCore.h`

**Constructor**:
```cpp
BendersCore(
    std::unique_ptr<IExecutionStrategy> execution,
    std::unique_ptr<IBatchingStrategy> batching,
    std::unique_ptr<IOuterLoopStrategy> outer_loop
);
```

**Key Methods**:

```cpp
// Main execution
void launch() override;

// Initialization
void InitializeProblems() override;

// Configuration
void set_input_map(const CouplingMap& coupling_map) override;

// Results
LogData GetBestIterationData() const override;
WorkerMasterDataVect AllCuts() const override;

// Metadata
std::string BendersName() const override;
double execution_time() const override;
```

**Orchestration Flow**:
```cpp
void BendersCore::launch() {
    // 1. Initialize outer loop
    if (outer_loop_) {
        outer_loop_->init_data();
    }
    
    // 2. Initialize batching
    if (batching_) {
        batching_->InitializeProblems();
    }
    
    // 3. Initialize execution
    if (execution_) {
        execution_->InitializeProblems();
    }
    
    // 4. Run (outer-loop controls if present)
    if (outer_loop_) {
        outer_loop_->Run(this);
    } else if (execution_) {
        execution_->Run();
    }
    
    // 5. Update stopping criterion
    if (batching_) {
        batching_->UpdateStoppingCriterion();
    }
}
```

**Example**:
```cpp
// Create strategies
auto exec = std::make_unique<SequentialExecutionStrategy>(/* ... */);
auto batch = std::make_unique<NoBatchingStrategy>();
auto outer = std::make_unique<NoOuterLoopStrategy>();

// Create orchestrator
auto core = std::make_unique<BendersCore>(
    std::move(exec),
    std::move(batch),
    std::move(outer)
);

// Use as IBendersCore
IBendersCore* benders = core.get();
benders->launch();
```

---

## Factory

### BendersFactory

**Purpose**: Creates BendersCore with appropriate strategies

**Location**: 
- Header: `src/cpp/benders/factories/include/antares-xpansion/benders/factories/BendersFactory.h`
- Implementation: `src/cpp/benders/factories/BendersFactory.cpp`

**Main Method**:
```cpp
BendersEnvironment PrepareForExecution(bool outer_loop);
```

**BendersEnvironment Structure**:
```cpp
struct BendersEnvironment {
    std::shared_ptr<IBendersCore> benders;  // The configured Benders instance
    std::shared_ptr<SolverAbstract> master;  // Master problem solver
};
```

**Strategy Selection Logic**:

1. **Execution Strategy** (automatic based on MPI world size):
   ```cpp
   if (world->size() == 1) {
       // Sequential
       execution = std::make_unique<SequentialExecutionStrategy>(...);
   } else {
       // MPI
       execution = std::make_unique<ParallelMpiExecutionStrategy>(...);
   }
   ```

2. **Batching Strategy** (based on BENDERSMETHOD):
   ```cpp
   if (method == BENDERS_BY_BATCH || method == BENDERS_BY_BATCH_OUTERLOOP) {
       batching = std::make_unique<ByBatchStrategy>(...);
   } else {
       batching = std::make_unique<NoBatchingStrategy>();
   }
   ```

3. **Outer-Loop Strategy** (based on parameter):
   ```cpp
   if (method == BENDERS_OUTERLOOP || method == BENDERS_BY_BATCH_OUTERLOOP) {
       outer_loop = std::make_unique<OuterLoopAdapter>(...);
   } else {
       outer_loop = std::make_unique<NoOuterLoopStrategy>();
   }
   ```

**Usage Example**:
```cpp
#include "antares-xpansion/benders/factories/BendersFactory.h"

// Create factory with dependencies
BendersFactory factory(dependencies);

// Get configured environment
auto env = factory.PrepareForExecution(outer_loop = true);

// env->benders is IBendersCore* (BendersCore instance)
env->benders->set_input_map(coupling_map);
env->benders->InitializeProblems();
env->benders->launch();

// Get results
auto results = env->benders->GetBestIterationData();
```

---

## Code Examples

### Example 1: Basic Usage

```cpp
#include "antares-xpansion/benders/factories/BendersFactory.h"

int main() {
    // Initialize dependencies
    BendersFactory::Dependencies deps = /* ... */;
    
    // Create factory
    BendersFactory factory(deps);
    
    // Get Benders environment
    auto env = factory.PrepareForExecution(outer_loop = false);
    
    // Use Benders
    env->benders->launch();
    
    // Get results
    auto best_iteration = env->benders->GetBestIterationData();
    
    return 0;
}
```

### Example 2: Custom Strategy Composition

```cpp
#include "antares-xpansion/benders/strategy/BendersCore.h"
#include "antares-xpansion/benders/strategy/SequentialExecutionStrategy.h"
#include "antares-xpansion/benders/strategy/ByBatchStrategy.h"
#include "antares-xpansion/benders/strategy/OuterLoopAdapter.h"

std::unique_ptr<IBendersCore> CreateCustomBenders() {
    // Create specific strategy combination
    auto sequential = std::make_unique<BendersSequential>(/* params */);
    auto exec = std::make_unique<SequentialExecutionStrategy>(
        std::move(sequential)
    );
    
    auto by_batch = std::make_unique<BendersByBatch>(/* params */);
    auto batch = std::make_unique<ByBatchStrategy>(
        std::move(by_batch)
    );
    
    auto outer_loop = std::make_unique<Outerloop::OuterLoop>(/* params */);
    auto outer = std::make_unique<OuterLoopAdapter>(
        std::move(outer_loop)
    );
    
    // Compose into BendersCore
    return std::make_unique<BendersCore>(
        std::move(exec),
        std::move(batch),
        std::move(outer)
    );
}
```

### Example 3: Testing with Mocks

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockExecutionStrategy : public IExecutionStrategy {
public:
    MOCK_METHOD(void, Run, (), (override));
    MOCK_METHOD(std::string, BendersName, (), (const, override));
    // ... other mocks
};

TEST(BendersCoreTest, ExecutesCorrectly) {
    // Create mock
    auto mock_exec = std::make_unique<MockExecutionStrategy>();
    
    // Set expectations
    EXPECT_CALL(*mock_exec, Run()).Times(1);
    EXPECT_CALL(*mock_exec, BendersName())
        .WillOnce(::testing::Return("MockExecution"));
    
    // Create BendersCore with mock
    auto core = std::make_unique<BendersCore>(
        std::move(mock_exec),
        std::make_unique<NoBatchingStrategy>(),
        std::make_unique<NoOuterLoopStrategy>()
    );
    
    // Test
    core->launch();
    EXPECT_EQ(core->BendersName(), "MockExecution");
}
```

### Example 4: Adding New Strategy

```cpp
// MyCustomExecutionStrategy.h
#pragma once
#include "antares-xpansion/benders/strategy/IExecutionStrategy.h"

class MyCustomExecutionStrategy : public IExecutionStrategy {
public:
    explicit MyCustomExecutionStrategy(/* params */) { /* ... */ }
    
    void Run() override {
        // Custom execution logic
    }
    
    void InitializeProblems() override {
        // Custom initialization
    }
    
    std::string BendersName() const override {
        return "MyCustomExecution";
    }
    
    // Implement all other IExecutionStrategy methods...
};

// Usage
auto custom = std::make_unique<MyCustomExecutionStrategy>(/* params */);
auto core = std::make_unique<BendersCore>(
    std::move(custom),
    std::make_unique<NoBatchingStrategy>(),
    std::make_unique<NoOuterLoopStrategy>()
);
core->launch();
```

---

## Type Compatibility

### IBendersCore vs. BendersBase

`IBendersCore` provides the same interface as the legacy `BendersBase`:

```cpp
// Legacy code
BendersBase* benders = /* factory */;
benders->launch();

// New code (compatible)
IBendersCore* benders = /* factory */;
benders->launch();  // Same method!
```

All methods from `BendersBase` are available in `IBendersCore`, ensuring backward compatibility.

---

## Summary

See [Architecture Overview](../architecture/benders-strategy-overview.md) for:
- Complete strategy hierarchy diagram
- Design principles and benefits
- Available strategy combinations table
