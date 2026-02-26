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

#### Key Strategy Interfaces

Three focused interfaces separate concerns:

- **IBendersCore**: Main interface for Benders engine operations
- **IExecutionStrategy**: Controls how subproblems are solved (Sequential vs. MPI)
- **IBatchingStrategy**: Controls problem batching
- **IOuterLoopStrategy**: Controls outer-loop optimization

For detailed interface signatures and methods, see [API Reference](../../api/benders-strategy-api.md)

## Component Details

### BendersCore (Orchestrator)

**Responsibility**: Compose strategies and orchestrate their execution

**Key Features**:
- Implements `IBendersCore` interface
- Holds three strategy instances (execution, batching, outer-loop)
- Delegates operations to appropriate strategy
- Coordinates execution flow

For implementation details and code examples, see [API Reference - BendersCore](../../api/benders-strategy-api.md#orchestrator)

### Execution Strategies

- **SequentialExecutionStrategy**: Single-process execution
- **ParallelMpiExecutionStrategy**: Multi-process MPI execution

Automatically selected based on `world->size()`. Details in [API Reference](../../api/benders-strategy-api.md#concrete-strategies)

### Batching Strategies

- **NoBatchingStrategy**: Process all subproblems together
- **ByBatchStrategy**: Process subproblems in batches

Selected based on BENDERSMETHOD enum. See [Developer Guide](../../developer-guide/benders-strategy-guide.md) for usage.

### Outer-Loop Strategies

- **NoOuterLoopStrategy**: Standard Benders without outer loop
- **OuterLoopAdapter**: Benders with outer-loop optimization

Selected based on BENDERSMETHOD enum. See [API Reference](../../api/benders-strategy-api.md) for details.

## Execution Flow

### Overview

Execution follows this sequence:
1. **Initialize outer-loop**: Set up data structures
2. **Initialize batching**: Configure batch processing
3. **Initialize execution**: Set up solver strategies
4. **Run**: Execute outer-loop (if enabled) or direct execution
5. **Update stopping criterion**: Check convergence

For detailed flow diagrams and code examples, see:
- [API Reference - Orchestrator](../../api/benders-strategy-api.md#orchestrator)
- [Developer Guide - Using the Strategy Pattern](../../developer-guide/benders-strategy-guide.md#using-the-strategy-pattern)

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

**Start here**: This is a high-level architecture overview.

**For detailed API information**: See [API Reference](../../api/benders-strategy-api.md)

**For practical guidance**: See [Developer Guide](../../developer-guide/benders-strategy-guide.md)

**For code location and navigation**: See [Code Navigation](../../developer-guide/code-navigation.md)

**For architectural decision context**: See [ADR 0001](adr/0001-benders-strategy-pattern.md)
