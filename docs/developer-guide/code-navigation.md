# Code Navigation Guide - Benders Strategy Pattern

## Table of Contents
1. [Directory Structure](#directory-structure)
2. [Key Files and Their Purposes](#key-files-and-their-purposes)
3. [Finding What You Need](#finding-what-you-need)
4. [Entry Points](#entry-points)
5. [Common Tasks](#common-tasks)

## Directory Structure

```
src/cpp/benders/
├── strategy/                          # Strategy pattern implementation
│   ├── include/antares-xpansion/benders/strategy/
│   │   ├── IBendersCore.h            # Main interface
│   │   ├── IExecutionStrategy.h      # Execution strategy interface
│   │   ├── IBatchingStrategy.h       # Batching strategy interface
│   │   ├── IOuterLoopStrategy.h      # Outer-loop strategy interface
│   │   ├── BendersCore.h             # Orchestrator implementation
│   │   ├── SequentialExecutionStrategy.h       # Sequential adapter
│   │   ├── ParallelMpiExecutionStrategy.h      # MPI adapter
│   │   ├── NoBatchingStrategy.h      # No-batching passthrough
│   │   ├── ByBatchStrategy.h         # Batching adapter
│   │   ├── NoOuterLoopStrategy.h     # No outer-loop passthrough
│   │   └── OuterLoopAdapter.h        # Outer-loop adapter
│   ├── src/                          # Strategy implementations (if needed)
│   │   └── BendersBaseCoreAdapter.*  # Legacy adapter
│   ├── tests/                        # Strategy tests
│   │   ├── SequentialExecutionStrategy_test.cpp
│   │   ├── ParallelMpiExecutionStrategy_test.cpp
│   │   ├── NoBatchingStrategy_test.cpp
│   │   ├── ByBatchStrategy_test.cpp
│   │   ├── NoOuterLoopStrategy_test.cpp
│   │   ├── OuterLoopAdapter_test.cpp
│   │   ├── BendersCore_test.cpp
│   │   └── BendersFactory_test.cpp
│   └── CMakeLists.txt                # Strategy build configuration
│
├── factories/                         # Factory pattern implementation
│   ├── include/antares-xpansion/benders/factories/
│   │   ├── BendersFactory.h          # Main factory class
│   │   └── BendersApp.h              # Application wrapper
│   ├── BendersFactory.cpp            # Factory implementation
│   └── CMakeLists.txt
│
├── benders_sequential/                # Sequential implementation (wrapped)
│   ├── include/.../BendersSequential.h
│   └── ...
│
├── benders_mpi/                       # MPI implementation (wrapped)
│   ├── include/.../BendersMpi.h
│   └── ...
│
├── benders_by_batch/                  # Batch implementation (wrapped)
│   ├── include/.../BendersByBatch.h
│   └── ...
│
└── benders_core/                      # Core/common code
    ├── include/.../BendersBase.h     # Base class (being wrapped)
    └── ...

docs/
├── architecture/
│   ├── adr/
│   │   └── 0001-benders-strategy-pattern.md    # ADR
│   └── benders-strategy-overview.md            # Architecture overview
├── developer-guide/
│   ├── benders-strategy-guide.md               # This guide
│   ├── code-navigation.md                      # Navigation guide
│   └── testing-strategy-pattern.md             # Testing guide
└── api/
    └── benders-strategy-api.md                 # API reference
```

## Key Files and Their Purposes

### Interfaces (What You Can Do)

| File | Purpose | When to Modify |
|------|---------|----------------|
| `IBendersCore.h` | Main Benders interface | Adding new Benders operations |
| `IExecutionStrategy.h` | Execution behavior | Adding execution-related operations |
| `IBatchingStrategy.h` | Batching behavior | Adding batching-related operations |
| `IOuterLoopStrategy.h` | Outer-loop behavior | Adding outer-loop operations |

### Implementations (How It's Done)

| File | Purpose | When to Use |
|------|---------|-------------|
| `BendersCore.h` | Orchestrates strategies | Changing execution flow |
| `SequentialExecutionStrategy.h` | Wraps BendersSequential | Sequential-specific changes |
| `ParallelMpiExecutionStrategy.h` | Wraps BendersMPI | MPI-specific changes |
| `NoBatchingStrategy.h` | No batching | Default batching behavior |
| `ByBatchStrategy.h` | Wraps BendersByBatch | Batching logic changes |
| `NoOuterLoopStrategy.h` | No outer loop | Default outer-loop behavior |
| `OuterLoopAdapter.h` | Wraps OuterLoop | Outer-loop logic changes |

### Factory and Creation

| File | Purpose | When to Modify |
|------|---------|----------------|
| `BendersFactory.h` | Factory interface | Adding factory methods |
| `BendersFactory.cpp` | Strategy creation | Strategy selection logic |
| `BendersApp.h` | Application entry point | Application-level changes |

### Legacy Code (Wrapped by Strategies)

| File | Purpose | Note |
|------|---------|------|
| `BendersBase.h` | Base class | Wrapped by strategies, don't modify |
| `BendersSequential.h` | Sequential impl | Used internally by strategy |
| `BendersMpi.h` | MPI impl | Used internally by strategy |
| `BendersByBatch.h` | Batching impl | Used internally by strategy |

## Finding What You Need

### "I want to add a new execution method"

1. **Create new strategy**: `src/cpp/benders/strategy/include/.../MyExecutionStrategy.h`
2. **Implement interface**: `IExecutionStrategy`
3. **Add tests**: `src/cpp/benders/strategy/tests/MyExecutionStrategy_test.cpp`
4. **Update factory**: `src/cpp/benders/factories/BendersFactory.cpp`

### "I want to modify how strategies are selected"

**File**: `src/cpp/benders/factories/BendersFactory.cpp`  
**Function**: `ConfigureBenders()`  
**Lines**: ~110-190

```cpp
// Example: Change selection logic
if (options.force_sequential || world_->size() == 1) {
    execution_strategy = std::make_unique<SequentialExecutionStrategy>(...);
} else {
    execution_strategy = std::make_unique<ParallelMpiExecutionStrategy>(...);
}
```

### "I want to understand the execution flow"

**Start at**: `BendersCore::launch()`  
**File**: `src/cpp/benders/strategy/include/.../BendersCore.h`  
**Lines**: ~50-75

Follow the orchestration:
1. `outer_loop_->init_data()`
2. `batching_->InitializeProblems()`
3. `execution_->InitializeProblems()`
4. `outer_loop_->Run()` or `execution_->Run()`
5. `batching_->UpdateStoppingCriterion()`

### "I want to add a new interface method"

1. **Add to interface**: e.g., `IExecutionStrategy.h`
2. **Update all implementations**:
   - `SequentialExecutionStrategy.h`
   - `ParallelMpiExecutionStrategy.h`
3. **Update BendersCore** if needed (delegation)
4. **Add tests** for each implementation

### "I want to see how tests are structured"

**Example test**: `src/cpp/benders/strategy/tests/SequentialExecutionStrategy_test.cpp`

Structure:
```cpp
// 1. Includes
#include "strategy header"
#include <gtest/gtest.h>

// 2. Mock class
class MockBendersSequential : public BendersSequential {
    MOCK_METHOD(...);
};

// 3. Test cases
TEST(StrategyTest, DelegatesMethod) {
    auto mock = std::make_unique<MockBendersSequential>();
    EXPECT_CALL(*mock, Method()).Times(1);
    
    Strategy strategy(std::move(mock));
    strategy.Method();
}
```

### "I want to add a new strategy combination"

1. **Check if strategies exist**: Do you need new implementations?
2. **Update factory logic**: Add selection criteria in `ConfigureBenders()`
3. **Add test**: Verify combination works in `BendersFactory_test.cpp`
4. **Document**: Update this guide and architecture docs

### "I want to debug a specific strategy"

**For Sequential**:
- Strategy: `SequentialExecutionStrategy.h`
- Wrapped: `BendersSequential.h` (in `benders_sequential/`)
- Test: `SequentialExecutionStrategy_test.cpp`

**For MPI**:
- Strategy: `ParallelMpiExecutionStrategy.h`
- Wrapped: `BendersMpi.h` (in `benders_mpi/`)
- Test: `ParallelMpiExecutionStrategy_test.cpp`

**For Batching**:
- Strategy: `ByBatchStrategy.h`
- Wrapped: `BendersByBatch.h` (in `benders_by_batch/`)
- Test: `ByBatchStrategy_test.cpp`

## Entry Points

### For Users

**File**: `src/cpp/benders/factories/BendersFactory.h`  
**Method**: `PrepareForExecution(bool outer_loop)`

```cpp
// Entry point for most users
auto env = factory.PrepareForExecution(outer_loop);
env->benders->launch();
```

### For Developers Adding Strategies

**File**: Create new strategy header  
**Location**: `src/cpp/benders/strategy/include/.../MyStrategy.h`

```cpp
#pragma once
#include "antares-xpansion/benders/strategy/IStrategy.h"

class MyStrategy : public IStrategy {
    // Implementation
};
```

### For Testing

**File**: Create new test file  
**Location**: `src/cpp/benders/strategy/tests/MyStrategy_test.cpp`

```cpp
#include <gtest/gtest.h>
TEST(MyStrategyTest, Works) { /* ... */ }
```

### For Build Configuration

**File**: `src/cpp/benders/strategy/CMakeLists.txt`

Add your strategy test:
```cmake
if(GTest_FOUND)
    add_executable(MyStrategy_test tests/MyStrategy_test.cpp)
    target_link_libraries(MyStrategy_test GTest::Main ...)
    add_test(NAME MyStrategy_test COMMAND MyStrategy_test)
endif()
```

## Common Tasks

### Task: Add a New Execution Strategy

**Files to modify**:
1. Create `MyExecutionStrategy.h` in `strategy/include/`
2. Create `MyExecutionStrategy_test.cpp` in `strategy/tests/`
3. Update `BendersFactory.cpp` in `factories/`
4. Update `CMakeLists.txt` in `strategy/`

**Steps**:
```bash
# 1. Create strategy header
cd src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/
vim MyExecutionStrategy.h

# 2. Create test
cd ../../tests/
vim MyExecutionStrategy_test.cpp

# 3. Update CMake
cd ..
vim CMakeLists.txt

# 4. Update factory
cd ../../factories/
vim BendersFactory.cpp

# 5. Build and test
cd /build
cmake ..
make MyExecutionStrategy_test
ctest -R MyExecutionStrategy_test
```

### Task: Modify Orchestration Logic

**File**: `src/cpp/benders/strategy/include/.../BendersCore.h`  
**Method**: `launch()`

```cpp
void launch() override {
    // Modify this sequence:
    if (outer_loop_) outer_loop_->init_data();
    if (batching_) batching_->InitializeProblems();
    if (execution_) execution_->InitializeProblems();
    
    // Add custom logic here
    
    if (outer_loop_) {
        outer_loop_->Run(this);
    } else if (execution_) {
        execution_->Run();
    }
    
    if (batching_) batching_->UpdateStoppingCriterion();
}
```

### Task: Change Strategy Selection Criteria

**File**: `src/cpp/benders/factories/BendersFactory.cpp`  
**Function**: `ConfigureBenders()`

```cpp
// Example: Add environment variable check
std::unique_ptr<IExecutionStrategy> CreateExecutionStrategy() {
    // New: Check environment variable
    const char* force_seq = std::getenv("FORCE_SEQUENTIAL");
    if (force_seq != nullptr) {
        return std::make_unique<SequentialExecutionStrategy>(...);
    }
    
    // Existing logic
    if (world_->size() == 1) {
        return std::make_unique<SequentialExecutionStrategy>(...);
    } else {
        return std::make_unique<ParallelMpiExecutionStrategy>(...);
    }
}
```

### Task: Debug Strategy Selection

Add debug output to factory:

```cpp
std::unique_ptr<IBendersCore> ConfigureBenders(...) {
    auto method = DeduceBendersMethod(...);
    
    // Debug output
    std::cout << "=== Strategy Selection ===" << std::endl;
    std::cout << "World size: " << world_->size() << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Batch size: " << options.BATCH_SIZE << std::endl;
    std::cout << "Outer loop: " << outer_loop << std::endl;
    
    // Create strategies...
    auto execution = CreateExecutionStrategy(...);
    std::cout << "Execution: " << execution->BendersName() << std::endl;
    
    // ...
}
```

### Task: Run Strategy Tests

```bash
# Run all strategy tests
cd build
ctest -R Strategy

# Run specific strategy test
ctest -R SequentialExecutionStrategy_test -V

# Run BendersCore tests
ctest -R BendersCore_test -V

# Run factory tests
ctest -R BendersFactory_test -V

# Run all tests
ctest --output-on-failure
```

## Tips and Tricks

### Finding Related Code

**Use grep to find usages**:
```bash
# Find all uses of IExecutionStrategy
grep -r "IExecutionStrategy" src/cpp/benders/

# Find all strategy creations
grep -r "make_unique.*Strategy" src/cpp/benders/

# Find all calls to a method
grep -r "->launch()" src/cpp/benders/
```

### Understanding Dependencies

**View dependency graph**:
```bash
# Strategy module depends on:
# - benders_core (BendersBase hierarchy)
# - benders_sequential (BendersSequential)
# - benders_mpi (BendersMPI)
# - benders_by_batch (BendersByBatch)
# - outer_loop (OuterLoop)
```

### Quick Reference

**Where is...**:
- Interfaces: `strategy/include/antares-xpansion/benders/strategy/I*.h`
- Implementations: `strategy/include/antares-xpansion/benders/strategy/*Strategy.h`
- Tests: `strategy/tests/*_test.cpp`
- Factory: `factories/BendersFactory.*`
- Documentation: `docs/`

**File naming conventions**:
- Interfaces: `I[Name]Strategy.h`
- Implementations: `[Name]Strategy.h`
- Tests: `[Name]_test.cpp`
- Mock classes: `Mock[Name]` (in test files)

## Next Steps

- Read [Developer Guide](benders-strategy-guide.md) for usage patterns
- See [Architecture Overview](../architecture/benders-strategy-overview.md) for design
- Review [API Reference](../api/benders-strategy-api.md) for interface details
