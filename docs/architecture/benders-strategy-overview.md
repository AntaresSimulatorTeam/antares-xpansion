# Benders Strategy Pattern - Architecture Overview

## Table of Contents
1. [Introduction](#introduction)
2. [High-Level Architecture](#high-level-architecture)
3. [Component Details](#component-details)
4. [Execution Flow](#execution-flow)
5. [Design Principles](#design-principles)
6. [Strategy Combinations](#strategy-combinations)

## Introduction

The Benders decomposition engine uses the **Strategy Pattern** to separate three independent concerns:
- **Execution**: How subproblems are solved (Sequential vs. Parallel MPI)
- **Batching**: How subproblems are grouped (No batching vs. By-batch)
- **Outer-Loop**: Whether outer-loop optimization is applied (No vs. Yes)

This architecture enables **8 different combinations** while maintaining clean separation of concerns and avoiding code duplication.

## High-Level Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     BendersFactory                          │
│  (Creates and configures BendersCore with strategies)       │
└────────────────────┬────────────────────────────────────────┘
                     │ creates
                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    BendersCore                              │
│         (Orchestrator - implements IBendersCore)            │
│                                                             │
│  ┌──────────────────┐  ┌──────────────────┐  ┌───────────┐│
│  │  Execution       │  │  Batching        │  │ OuterLoop ││
│  │  Strategy        │  │  Strategy        │  │ Strategy  ││
│  └──────────────────┘  └──────────────────┘  └───────────┘│
└────────────┬────────────────┬─────────────────┬────────────┘
             │                │                 │
             ▼                ▼                 ▼
    ┌────────────────┐ ┌─────────────┐ ┌──────────────┐
    │IExecution      │ │IBatching    │ │IOuterLoop    │
    │Strategy        │ │Strategy     │ │Strategy      │
    └────────────────┘ └─────────────┘ └──────────────┘
             │                │                 │
      ┌──────┴──────┐  ┌──────┴──────┐  ┌──────┴──────┐
      │             │  │             │  │             │
      ▼             ▼  ▼             ▼  ▼             ▼
┌──────────┐ ┌──────────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐
│Sequential│ │Parallel  │ │No    │ │By    │ │No    │ │Outer │
│Execution │ │Mpi       │ │Batch │ │Batch │ │Outer │ │Loop  │
│Strategy  │ │Execution │ │      │ │      │ │Loop  │ │      │
│          │ │Strategy  │ │      │ │      │ │      │ │      │
└──────────┘ └──────────┘ └──────┘ └──────┘ └──────┘ └──────┘
     │             │           │        │        │        │
     └─────┬───────┘           └────┬───┘        └────┬───┘
           │ wraps                  │ wraps           │ wraps
           ▼                        ▼                 ▼
    ┌──────────────┐        ┌──────────────┐  ┌──────────┐
    │ BendersBase  │        │BendersByBatch│  │OuterLoop │
    │ Hierarchy    │        │              │  │          │
    │              │        └──────────────┘  └──────────┘
    │ - Sequential │
    │ - MPI        │
    └──────────────┘
```

### Key Abstractions

#### IBendersCore Interface
The main interface for Benders engine operations:
```cpp
class IBendersCore {
public:
    virtual void launch() = 0;
    virtual void InitializeProblems() = 0;
    virtual void set_input_map(const CouplingMap&) = 0;
    virtual int MasterRowIndex(const std::string&) const = 0;
    virtual LogData GetBestIterationData() const = 0;
    virtual WorkerMasterDataVect AllCuts() const = 0;
    // ... other methods
};
```

#### Strategy Interfaces

**IExecutionStrategy**: Controls how subproblems are solved
```cpp
class IExecutionStrategy {
public:
    virtual void Run() = 0;
    virtual void InitializeProblems() = 0;
    virtual void set_input_map(const CouplingMap&) = 0;
    virtual std::string BendersName() const = 0;
    // ... other methods
};
```

**IBatchingStrategy**: Controls problem batching
```cpp
class IBatchingStrategy {
public:
    virtual void InitializeProblems() = 0;
    virtual void UpdateStoppingCriterion() = 0;
    virtual bool ShouldRelaxationStop() const = 0;
};
```

**IOuterLoopStrategy**: Controls outer-loop optimization
```cpp
class IOuterLoopStrategy {
public:
    virtual void Run(IBendersCore*) = 0;
    virtual void init_data() = 0;
    virtual bool UpdateMaster(WorkerMasterDataVect&) = 0;
    // ... other methods
};
```

## Component Details

### BendersCore (Orchestrator)

**Responsibility**: Compose strategies and orchestrate their execution

**Key Features**:
- Implements `IBendersCore` interface
- Holds three strategy instances (execution, batching, outer-loop)
- Delegates operations to appropriate strategy
- Coordinates execution flow

**Location**: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/BendersCore.h`

**Example**:
```cpp
class BendersCore : public IBendersCore {
public:
    BendersCore(
        std::unique_ptr<IExecutionStrategy> exec,
        std::unique_ptr<IBatchingStrategy> batch,
        std::unique_ptr<IOuterLoopStrategy> outer
    );
    
    void launch() override;
    // ... other IBendersCore methods
    
private:
    std::unique_ptr<IExecutionStrategy> execution_;
    std::unique_ptr<IBatchingStrategy> batching_;
    std::unique_ptr<IOuterLoopStrategy> outer_loop_;
};
```

### Execution Strategies

#### SequentialExecutionStrategy
**Wraps**: BendersSequential  
**Use Case**: Single-process execution  
**Selection**: Automatic when `world->size() == 1`  
**Location**: `src/cpp/benders/strategy/include/.../SequentialExecutionStrategy.h`

#### ParallelMpiExecutionStrategy
**Wraps**: BendersMPI  
**Use Case**: Multi-process MPI execution  
**Selection**: Automatic when `world->size() > 1`  
**Location**: `src/cpp/benders/strategy/include/.../ParallelMpiExecutionStrategy.h`

### Batching Strategies

#### NoBatchingStrategy
**Behavior**: Passthrough (no batching logic)  
**Use Case**: Process all subproblems together  
**Selection**: When BENDERSMETHOD doesn't include "BY_BATCH"  
**Location**: `src/cpp/benders/strategy/include/.../NoBatchingStrategy.h`

#### ByBatchStrategy
**Wraps**: BendersByBatch  
**Use Case**: Process subproblems in batches  
**Selection**: When BENDERSMETHOD includes "BY_BATCH"  
**Location**: `src/cpp/benders/strategy/include/.../ByBatchStrategy.h`

### Outer-Loop Strategies

#### NoOuterLoopStrategy
**Behavior**: Passthrough (no outer-loop optimization)  
**Use Case**: Standard Benders without outer loop  
**Selection**: When BENDERSMETHOD doesn't include "OUTERLOOP"  
**Location**: `src/cpp/benders/strategy/include/.../NoOuterLoopStrategy.h`

#### OuterLoopAdapter
**Wraps**: Outerloop::OuterLoop  
**Use Case**: Benders with outer-loop optimization  
**Selection**: When BENDERSMETHOD includes "OUTERLOOP"  
**Location**: `src/cpp/benders/strategy/include/.../OuterLoopAdapter.h`

## Execution Flow

### Initialization Flow

```
BendersFactory::PrepareForExecution()
    │
    ├─→ Determine BENDERSMETHOD (from options)
    │
    ├─→ ConfigureBenders()
    │   │
    │   ├─→ Create ExecutionStrategy
    │   │   └─→ world->size() == 1 ? Sequential : MPI
    │   │
    │   ├─→ Create BatchingStrategy
    │   │   └─→ method has BY_BATCH ? ByBatch : NoBatch
    │   │
    │   ├─→ Create OuterLoopStrategy
    │   │   └─→ method has OUTERLOOP ? OuterLoop : None
    │   │
    │   └─→ Create BendersCore(exec, batch, outer)
    │
    └─→ Return BendersEnvironment { benders: IBendersCore* }
```

### Runtime Execution Flow

```
client->launch()  // Called on IBendersCore
    │
    ▼
BendersCore::launch()
    │
    ├─→ 1. outer_loop_->init_data()
    │
    ├─→ 2. batching_->InitializeProblems()
    │
    ├─→ 3. execution_->InitializeProblems()
    │
    ├─→ 4. if (outer_loop_ != nullptr)
    │   │     outer_loop_->Run(this)  // Outer loop controls execution
    │   │
    │   └─→ else
    │         execution_->Run()        // Direct execution
    │
    └─→ 5. batching_->UpdateStoppingCriterion()
```

### Delegation Examples

**Example 1: Get master row index**
```
Client → BendersCore::MasterRowIndex()
           │
           └─→ execution_->MasterRowIndex()
                  │
                  └─→ wrapped_benders_->MasterRowIndex()
```

**Example 2: Check if should stop**
```
Client → BendersCore::ShouldRelaxationStop()
           │
           └─→ batching_->ShouldRelaxationStop()
```

## Design Principles

### 1. Composition Over Inheritance
- BendersCore **composes** strategies rather than inheriting from BendersBase
- Strategies can be swapped at runtime
- No diamond problem, no deep hierarchies

### 2. Single Responsibility Principle
- **ExecutionStrategy**: Only handles execution (Sequential vs. MPI)
- **BatchingStrategy**: Only handles batching logic
- **OuterLoopStrategy**: Only handles outer-loop optimization
- **BendersCore**: Only orchestrates

### 3. Open/Closed Principle
- Adding a new strategy doesn't require modifying existing code
- Just create new class implementing the interface
- Factory can be extended to use new strategies

### 4. Dependency Injection
- Strategies injected into BendersCore via constructor
- No hard dependencies on concrete classes
- Easy to test with mocks

### 5. Interface Segregation
- Three focused interfaces instead of one monolithic interface
- Clients depend only on what they use
- Strategies implement only relevant interface

## Strategy Combinations

All **8 combinations** are supported:

| Execution | Batching | OuterLoop | BENDERSMETHOD         |
|-----------|----------|-----------|----------------------|
| Sequential| NoBatch  | None      | BENDERS              |
| Sequential| NoBatch  | OuterLoop | BENDERS_OUTERLOOP    |
| Sequential| ByBatch  | None      | BENDERS_BY_BATCH     |
| Sequential| ByBatch  | OuterLoop | BENDERS_BY_BATCH_OL  |
| MPI       | NoBatch  | None      | BENDERS              |
| MPI       | NoBatch  | OuterLoop | BENDERS_OUTERLOOP    |
| MPI       | ByBatch  | None      | BENDERS_BY_BATCH     |
| MPI       | ByBatch  | OuterLoop | BENDERS_BY_BATCH_OL  |

**Note**: Execution strategy (Sequential vs. MPI) is automatically selected based on `world->size()`.

## Benefits Summary

### For Developers
- ✅ Clear separation of concerns
- ✅ Easy to add new strategies
- ✅ Each component testable in isolation
- ✅ Flexible runtime composition

### For Maintainability
- ✅ No code duplication
- ✅ Changes isolated to relevant strategy
- ✅ Easier to understand (each file does one thing)
- ✅ Better test coverage (66 tests)

### For Performance
- ✅ No runtime overhead (header-only strategies)
- ✅ Same performance as original (strategies wrap existing code)
- ✅ Optimizations can target specific strategies

### For Users
- ✅ No API changes
- ✅ Backward compatible
- ✅ Same usage patterns
- ✅ More combinations available

## Next Steps

For detailed information, see:
- **Developer Guide**: `docs/developer-guide/benders-strategy-guide.md`
- **Code Navigation**: `docs/developer-guide/code-navigation.md`
- **Testing Guide**: `docs/developer-guide/testing-strategy-pattern.md`
- **API Reference**: `docs/api/benders-strategy-api.md`
- **ADR**: `docs/architecture/adr/0001-benders-strategy-pattern.md`
