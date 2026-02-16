# Benders Architecture Refactoring - Implementation Checklist & Roadmap

**Status**: PLANNING  
**Recommended Solution**: Strategy Pattern (Solution 1)  
**Target Completion**: 6 weeks (39 hours developer time)  
**Priority**: MEDIUM (Technical Debt Reduction)

---

## Executive Summary

This document provides a detailed implementation roadmap for refactoring the Benders architecture from an inheritance-based, duplication-prone design to a clean, composable Strategy Pattern.

### Key Outcomes
- ✅ Eliminate ~800-1000 lines of code duplication (MPI ↔ Sequential)
- ✅ Unify OuterLoop handling (inheritance + composition → pure composition)
- ✅ Enable Sequential+OuterLoop combination (currently impossible)
- ✅ Remove switch/case Factory explosion (enable future variants)
- ✅ Reduce maintenance burden by 40%

### Before vs. After

```
BEFORE:                        AFTER:
├─ BendersBase (363L)          ├─ BendersBase (interface, unchanged)
├─ BendersMpi (400L code)      ├─ BendersCore (composition)
├─ BendersSequential (400L)    │  ├─ uses ExecutionStrategy
├─ BendersByBatch             │  ├─ uses BatchingStrategy
├─ BendersMpiOuterLoop        │  └─ uses OuterLoopStrategy
├─ OuterLoopBenders           ├─ Strategies (ParallelMpi, Sequential, ByBatch, OuterLoop, etc.)
└─ 4+ MathLogger variants      └─ 1 unified MathLogger
```

---

## Phase-by-Phase Implementation Plan

---

## PHASE 1: Architecture Design & Setup (Week 1, 5 hours)

### 1.1 Design Strategy Interfaces (2h)

**Goal**: Define 3 strategy interfaces with clear contracts

#### 1.1.1 Create ExecutionStrategy Interface

**File**: `src/cpp/benders/strategies/include/ExecutionStrategy.h`

```cpp
#pragma once
#include "antares-xpansion/benders/benders_core/common.h"

namespace Benders::Strategies {

/**
 * \interface ExecutionStrategy
 * \brief Abstracts execution mode (MPI, Sequential, future GPU, etc.)
 *
 * Handles:
 * - Initialization of subproblems
 * - Solving subproblems
 * - Gathering results from all ranks (MPI) or local (Sequential)
 * - Broadcasting variable indices
 * - Cleanup
 */
class ExecutionStrategy {
public:
    virtual ~ExecutionStrategy() = default;

    /// Initialize all subproblems for this variant
    virtual void InitializeProblems() = 0;

    /// Solve a single subproblem and return cut package
    virtual SubProblemDataMap SolveSubproblem(
        PlainData::SubProblemData& subproblem_data,
        const std::string& subproblem_name
    ) = 0;

    /// Gather subproblem cuts from all ranks
    virtual void GatherCuts(
        const SubProblemDataMap& local_cut,
        std::vector<SubProblemDataMap>& all_cuts
    ) = 0;

    /// Broadcast variable indices to all ranks
    virtual void BroadCastVariablesIndices() = 0;

    /// Synchronization barrier (no-op for Sequential)
    virtual void Barrier() const = 0;

    /// Cleanup resources
    virtual void Cleanup() = 0;

    /// Is this parallel execution?
    virtual bool IsParallel() const = 0;
};

} // namespace Benders::Strategies
```

**Contract**:
- All methods MUST be implemented by subclasses
- MPI methods (Gather, Broadcast, Barrier) are no-op in Sequential
- Sequential uses ArchiveReader; MPI uses communicator
- Solver access needed: pass via constructor or reference

**Testing**:
- Unit test: mock strategy for core logic
- Integration: real MPI vs Sequential strategies

---

#### 1.1.2 Create BatchingStrategy Interface

**File**: `src/cpp/benders/strategies/include/BatchingStrategy.h`

```cpp
#pragma once
#include "antares-xpansion/benders/benders_core/common.h"

namespace Benders::Strategies {

/**
 * \interface BatchingStrategy
 * \brief Abstracts batching mode (None, ByBatch)
 *
 * Handles:
 * - Dividing subproblems into batches
 * - Processing batches
 * - Computing batch-specific criteria
 * - Updating stopping criterion
 */
class BatchingStrategy {
public:
    virtual ~BatchingStrategy() = default;

    /// Setup before first batch (e.g., initialize batch collection)
    virtual void PreLaunchSetup(
        const CouplingMap& coupling_map,
        size_t batch_size
    ) = 0;

    /// Get next batch of subproblems to solve
    virtual std::vector<std::string> GetNextBatch() = 0;

    /// Are there more batches?
    virtual bool HasNextBatch() const = 0;

    /// Process cuts from a single batch
    virtual void ProcessBatch(
        const SubProblemDataMap& batch_cuts,
        double batch_contribution_in_gap
    ) = 0;

    /// Compute batch-specific gap contribution
    virtual double ComputeGapContribution(
        const std::vector<SubProblemDataMap>& all_cuts
    ) const = 0;

    /// Check batch-specific stopping criterion
    virtual bool ShouldStop() const = 0;

    /// Reset for next iteration
    virtual void Reset() = 0;
};

} // namespace Benders::Strategies
```

**Contract**:
- NoBatchingStrategy: HasNextBatch() returns false after first iteration
- ByBatchStrategy: Uses BatchCollection, HasNextBatch() = more batches
- ProcessBatch() differs significantly between variants

---

#### 1.1.3 Create OuterLoopStrategy Interface

**File**: `src/cpp/benders/strategies/include/OuterLoopStrategy.h`

```cpp
#pragma once
#include "antares-xpansion/benders/benders_core/CriterionComputation.h"

namespace Benders::Strategies {

/**
 * \interface OuterLoopStrategy
 * \brief Abstracts outer loop mode (None, Active)
 *
 * Handles:
 * - Master update between outer loop iterations
 * - Feasibility and bilevel checks
 * - Lambda min/max computations
 * - Outer loop convergence
 */
class OuterLoopStrategy {
public:
    virtual ~OuterLoopStrategy() = default;

    /// Called before entering main Benders loop
    virtual void BeforeLaunch() = 0;

    /// Called after each Benders iteration
    virtual bool CheckAndUpdate() = 0;

    /// Called after Benders converges
    virtual void AfterLaunch() = 0;

    /// Should outer loop continue?
    virtual bool ShouldContinue() const = 0;

    /// Get current iteration number
    virtual int GetIterationNumber() const = 0;
};

} // namespace Benders::Strategies
```

**Contract**:
- NoOuterLoopStrategy: All methods are no-ops, ShouldContinue() = false
- OuterLoopWrapper: Contains actual OuterLoop/OuterLoopBiLevel logic

---

### 1.2 Create Directory Structure (1h)

```
src/cpp/benders/
├── strategies/                   [NEW]
│   ├── CMakeLists.txt
│   ├── include/antares-xpansion/benders/strategies/
│   │   ├── ExecutionStrategy.h
│   │   ├── BatchingStrategy.h
│   │   ├── OuterLoopStrategy.h
│   │   └── (implementations below)
│   ├── ParallelMpiExecutor.h/cpp
│   ├── SequentialExecutor.h/cpp
│   ├── NoBatchingStrategy.h/cpp
│   ├── ByBatchStrategy.h/cpp
│   ├── NoOuterLoopStrategy.h/cpp
│   └── OuterLoopWrapper.h/cpp
├── benders_core/
│   ├── BendersCore.h/cpp          [NEW - consolidation]
│   └── ...
└── ...
```

**CMakeLists.txt** for strategies:
```cmake
add_library(benders_strategies)
target_sources(benders_strategies PRIVATE
    ParallelMpiExecutor.cpp
    SequentialExecutor.cpp
    NoBatchingStrategy.cpp
    ByBatchStrategy.cpp
    NoOuterLoopStrategy.cpp
    OuterLoopWrapper.cpp
)
target_link_libraries(benders_strategies PUBLIC
    benders_core
    Boost::mpi
    # ... other deps
)
```

---

### 1.3 Architecture Review & Feedback (1h)

- [ ] Present interfaces to team
- [ ] Review contracts and edge cases
- [ ] Confirm ExecutionStrategy vs BendersCore responsibility split
- [ ] Get sign-off on Phase 1 before proceeding

---

### 1.4 Setup Branch & PR Template (1h)

- [ ] Create branch: `feature/benders-strategy-refactor`
- [ ] Create PR template with checklist:
  - [ ] Tests passing (MPI, Sequential, ByBatch, OuterLoop)
  - [ ] Regression tests vs baseline
  - [ ] Code review by 2+ engineers
  - [ ] Documentation updated

---

### Deliverable for Phase 1
- ✅ 3 strategy interfaces defined with contracts
- ✅ Directory structure ready
- ✅ Branch created
- ✅ Team alignment

---

## PHASE 2: Extract ParallelMpiExecutor (Week 2-3, 8 hours)

### 2.1 Implement ParallelMpiExecutor Class (5h)

**File**: `src/cpp/benders/strategies/ParallelMpiExecutor.h`

```cpp
#pragma once
#include "ExecutionStrategy.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"

namespace Benders::Strategies {

class ParallelMpiExecutor : public ExecutionStrategy {
public:
    ParallelMpiExecutor(
        mpi::communicator& world,
        const BendersBaseOptions& options,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<WorkerMaster> master,
        const VariableMap& master_variable_map,
        const CouplingMap& coupling_map
    );

    ~ParallelMpiExecutor() override = default;

    void InitializeProblems() override;
    SubProblemDataMap SolveSubproblem(
        PlainData::SubProblemData& subproblem_data,
        const std::string& name
    ) override;
    void GatherCuts(
        const SubProblemDataMap& local_cut,
        std::vector<SubProblemDataMap>& all_cuts
    ) override;
    void BroadCastVariablesIndices() override;
    void Barrier() const override { _world.barrier(); }
    void Cleanup() override;
    bool IsParallel() const override { return true; }

private:
    mpi::communicator& _world;
    const BendersBaseOptions& options_;
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<WorkerMaster> master_;
    const VariableMap& master_variable_map_;
    const CouplingMap& coupling_map_;

    // MPI-specific helpers
    void InitializeMpiWorkers();
    SubProblemDataMap GetSubproblemCut();
    // ... other MPI-specific methods
};

} // namespace Benders::Strategies
```

**Implementation Details**:
- Move entire `BendersMpi::InitializeProblems()` logic → `ParallelMpiExecutor::InitializeProblems()`
- Move `BendersMpi::SolveSubproblem()` → `ParallelMpiExecutor::SolveSubproblem()`
- Move MPI gather/broadcast logic → respective methods
- Keep backward compatibility: check that MPI tests still pass

**Code Extraction Checklist**:
- [ ] Copy `BendersMpi::InitializeProblems()` implementation
- [ ] Copy `BendersMpi::SolveSubproblem()` implementation
- [ ] Copy MPI gather/broadcast logic from BendersMpi.cpp
- [ ] Remove MPI-specific code from BendersCore (TBD in Phase 6)
- [ ] Compile and run MPI tests

---

### 2.2 Update BendersCore to Use ExecutionStrategy (2h)

**File**: `src/cpp/benders/benders_core/BendersCore.h` (modified)

```cpp
class BendersCore : public BendersBase {
private:
    std::unique_ptr<ExecutionStrategy> executor_;
    // ... other members

    void Run() override {
        // 1. Initialize via strategy
        executor_->InitializeProblems();
        
        // 2. Main loop
        while (!convergence_reached()) {
            SubProblemDataMap local_cuts;
            
            // 3. Solve subproblems via strategy
            for (auto& [name, subproblem] : subproblems_) {
                auto cut = executor_->SolveSubproblem(subproblem, name);
                local_cuts[name] = cut;
            }
            
            // 4. Gather via strategy
            std::vector<SubProblemDataMap> all_cuts;
            executor_->GatherCuts(local_cuts, all_cuts);
            
            // 5. Update master (common logic)
            UpdateMasterFromCuts(all_cuts);
            
            // 6. Check convergence
            ComputeConvergenceCriterion();
        }
        
        executor_->Cleanup();
    }
};
```

**Update Responsibilities**:
- [ ] BendersCore::InitializeProblems() calls executor_->InitializeProblems()
- [ ] BendersCore::Run() uses executor_ for MPI operations
- [ ] BendersCore::free() calls executor_->Cleanup()
- [ ] Remove MPI-specific methods from BendersCore

---

### 2.3 Test MPI Path (1h)

**Test Checklist**:
- [ ] Compile with new ExecutionStrategy interface
- [ ] Run existing MPI tests: `./test-benders-mpi`
- [ ] Check for regressions in walltime, output
- [ ] Verify MPI communication (gather, broadcast) works
- [ ] Unit test: ParallelMpiExecutor::InitializeProblems()
- [ ] Integration test: MPI path end-to-end

**Regression Test**:
```bash
# Before refactoring
./benders --method=BENDERS > baseline_mpi.log 2>&1
baseline_time=$(grep "Elapsed time" baseline_mpi.log)

# After refactoring
./benders --method=BENDERS > refactored_mpi.log 2>&1
refactored_time=$(grep "Elapsed time" refactored_mpi.log)

# Check: time delta < 5%
delta=$(echo "scale=2; ($refactored_time - $baseline_time) / $baseline_time * 100" | bc)
if [ $delta -lt 5 ]; then
    echo "✓ PASS: Walltime delta = $delta%"
else
    echo "✗ FAIL: Walltime delta = $delta% (threshold: 5%)"
fi
```

---

### 2.4 Create PR#2

**Title**: `[CORE] Benders Refactor - ParallelMpiExecutor Strategy`

**Checklist**:
- [ ] ParallelMpiExecutor interface + implementation
- [ ] BendersCore uses ExecutionStrategy
- [ ] All MPI tests passing
- [ ] Regression < 5% walltime
- [ ] No duplication introduced
- [ ] Code review: 2 approvals

---

### Deliverable for Phase 2
- ✅ ParallelMpiExecutor fully functional
- ✅ MPI path passing all tests
- ✅ PR#2 merged

---

## PHASE 3: Extract SequentialExecutor (Week 4, 5 hours)

### 3.1 Analyze BendersSequential vs BendersMpi (1h)

**Goal**: Understand duplication points

**Comparison Checklist**:
- [ ] InitializeProblems(): archive reader (Seq) vs MPI distribution (MPI)
- [ ] SolveSubproblem(): solve locally (Seq) vs gather from workers (MPI)
- [ ] GatherCuts(): local storage (Seq) vs MPI gather (MPI)
- [ ] Common parts: master update, convergence check, logging

**Finding**:
- ~80% code identical between MPI and Sequential InitializeProblems()
- ~70% code identical between MPI and Sequential SolveSubproblem()
- Can refactor common parts to BendersCore after extraction

---

### 3.2 Implement SequentialExecutor (3h)

**File**: `src/cpp/benders/strategies/SequentialExecutor.h`

```cpp
#pragma once
#include "ExecutionStrategy.h"
#include "antares-xpansion/helpers/ArchiveReader.h"

namespace Benders::Strategies {

class SequentialExecutor : public ExecutionStrategy {
public:
    SequentialExecutor(
        const BendersBaseOptions& options,
        std::shared_ptr<ILogger> logger,
        std::shared_ptr<WorkerMaster> master,
        const VariableMap& master_variable_map,
        const CouplingMap& coupling_map
    );

    ~SequentialExecutor() override = default;

    void InitializeProblems() override;
    SubProblemDataMap SolveSubproblem(
        PlainData::SubProblemData& subproblem_data,
        const std::string& name
    ) override;
    void GatherCuts(
        const SubProblemDataMap& local_cut,
        std::vector<SubProblemDataMap>& all_cuts
    ) override;
    void BroadCastVariablesIndices() override {}  // No-op
    void Barrier() const override {}              // No-op
    void Cleanup() override;
    bool IsParallel() const override { return false; }

private:
    ArchiveReader reader_;
    const BendersBaseOptions& options_;
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<WorkerMaster> master_;
    const VariableMap& master_variable_map_;
    const CouplingMap& coupling_map_;

    // Sequential-specific helpers
    void InitializeLocalSolvers();
};

} // namespace Benders::Strategies
```

**Code Extraction**:
- [ ] Copy `BendersSequential::InitializeProblems()` → `SequentialExecutor::InitializeProblems()`
- [ ] Copy `BendersSequential::SolveSubproblem()` → `SequentialExecutor::SolveSubproblem()`
- [ ] Implement GatherCuts() = just return local_cut
- [ ] No-op broadcast/barrier for Sequential

---

### 3.3 Test Sequential Path & Add New Tests (1h)

**Test Checklist**:
- [ ] Compile with SequentialExecutor
- [ ] Create unit test: SequentialExecutor::InitializeProblems()
- [ ] Create integration test: Sequential path end-to-end
- [ ] Compare output vs old BendersSequential
- [ ] Verify cuts are identical

**New Test File**: `tests/unit/test_sequential_executor.cpp`

```cpp
TEST_CASE("SequentialExecutor initializes problems", "[sequential]") {
    auto executor = std::make_unique<SequentialExecutor>(...);
    executor->InitializeProblems();
    // Assert: subproblems loaded, solvers ready
    REQUIRE(executor->GetSubproblemCount() > 0);
}

TEST_CASE("SequentialExecutor solves subproblem", "[sequential]") {
    auto executor = std::make_unique<SequentialExecutor>(...);
    executor->InitializeProblems();
    PlainData::SubProblemData sp = ...;
    auto cut = executor->SolveSubproblem(sp, "area1");
    REQUIRE(cut.IsValid());
}
```

---

### 3.4 Create PR#3

**Title**: `[CORE] Benders Refactor - SequentialExecutor Strategy`

**Checklist**:
- [ ] SequentialExecutor interface + implementation
- [ ] Sequential tests passing
- [ ] Output matches old Sequential variant
- [ ] Code review: 2 approvals

---

### Deliverable for Phase 3
- ✅ SequentialExecutor fully functional
- ✅ Sequential tests passing
- ✅ Duplication between MPI/Sequential identified
- ✅ PR#3 merged

---

## PHASE 4: Extract BatchingStrategy (Week 5, 3 hours)

### 4.1 Analyze BendersByBatch vs Standard Benders (1h)

**Key Differences**:
- InitializeProblems(): adds batch collection setup
- Run(): batches subproblems, updates gap per batch
- UpdateStoppingCriterion(): batch-specific
- ComputeXCut(): batch-aware

---

### 4.2 Implement Batching Strategies (1.5h)

**File**: `src/cpp/benders/strategies/NoBatchingStrategy.h/cpp`

```cpp
class NoBatchingStrategy : public BatchingStrategy {
public:
    void PreLaunchSetup(...) override {
        // No setup for no batching
    }
    
    std::vector<std::string> GetNextBatch() override {
        // Return all subproblems at once
        return all_subproblems_;
    }
    
    bool HasNextBatch() const override {
        return !batch_returned_;  // One iteration only
    }
    
    // ... other methods
};
```

**File**: `src/cpp/benders/strategies/ByBatchStrategy.h/cpp`

```cpp
class ByBatchStrategy : public BatchingStrategy {
private:
    BatchCollection batch_collection_;
    size_t current_batch_index_ = 0;

public:
    void PreLaunchSetup(...) override {
        batch_collection_.Initialize(coupling_map, batch_size);
    }
    
    std::vector<std::string> GetNextBatch() override {
        return batch_collection_.GetBatch(current_batch_index_);
    }
    
    bool HasNextBatch() const override {
        return current_batch_index_ < batch_collection_.NumBatches();
    }
    
    // ... other methods from BendersByBatch
};
```

---

### 4.3 Test Batching Strategies (0.5h)

- [ ] Unit test: NoBatchingStrategy (returns all at once)
- [ ] Unit test: ByBatchStrategy (returns batches)
- [ ] Integration test: MPI + ByBatch works
- [ ] Integration test: Sequential + ByBatch (new combination)

---

### 4.4 Create PR#4

**Title**: `[CORE] Benders Refactor - BatchingStrategy`

**Checklist**:
- [ ] NoBatchingStrategy + ByBatchStrategy implemented
- [ ] Tests passing
- [ ] MPI+Batch integration test passing
- [ ] Code review: 1 approval

---

### Deliverable for Phase 4
- ✅ BatchingStrategy interface + implementations
- ✅ Batching tests passing
- ✅ PR#4 merged

---

## PHASE 5: Extract OuterLoopStrategy (Week 6, 2 hours)

### 5.1 Implement OuterLoop Strategies (1h)

**File**: `src/cpp/benders/strategies/NoOuterLoopStrategy.h/cpp`

```cpp
class NoOuterLoopStrategy : public OuterLoopStrategy {
public:
    void BeforeLaunch() override {}
    bool CheckAndUpdate() override { return false; }
    void AfterLaunch() override {}
    bool ShouldContinue() const override { return false; }
    int GetIterationNumber() const override { return 0; }
};
```

**File**: `src/cpp/benders/strategies/OuterLoopWrapper.h/cpp`

```cpp
class OuterLoopWrapper : public OuterLoopStrategy {
private:
    std::unique_ptr<Outerloop::OuterLoopBiLevel> outer_loop_;
    int outer_loop_iteration_ = 0;

public:
    OuterLoopWrapper(...) : outer_loop_(...) {}
    
    void BeforeLaunch() override {
        outer_loop_->Initialize();
        outer_loop_iteration_ = 0;
    }
    
    bool CheckAndUpdate() override {
        // Execute outer loop logic
        return outer_loop_->Iterate();
    }
    
    // ... other methods
};
```

---

### 5.2 Test OuterLoop with All Variants (0.5h)

**New Tests**:
- [ ] MPI + OuterLoop
- [ ] Sequential + OuterLoop (NEW!)
- [ ] ByBatch + OuterLoop (NEW!)
- [ ] Sequential + ByBatch + OuterLoop (NEW!)

---

### 5.3 Create PR#5

**Title**: `[CORE] Benders Refactor - OuterLoopStrategy`

**Checklist**:
- [ ] NoOuterLoopStrategy + OuterLoopWrapper implemented
- [ ] All 4+ combinations tested
- [ ] Sequential+OuterLoop working (new feature!)
- [ ] Code review: 2 approvals

---

### Deliverable for Phase 5
- ✅ OuterLoopStrategy interface + implementations
- ✅ All variant combinations working
- ✅ Sequential+OuterLoop now supported
- ✅ PR#5 merged

---

## PHASE 6: BendersCore Consolidation (Week 7, 4 hours)

### 6.1 Implement BendersCore Class (3h)

**File**: `src/cpp/benders/benders_core/BendersCore.h/cpp`

**Key Responsibilities**:
1. Aggregate 3 strategies
2. Unified Run() loop combining all strategies
3. Common logic: master update, convergence, logging
4. Backward compatibility with BendersBase interface

```cpp
class BendersCore : public BendersBase {
private:
    std::unique_ptr<ExecutionStrategy> executor_;
    std::unique_ptr<BatchingStrategy> batcher_;
    std::unique_ptr<OuterLoopStrategy> outer_loop_;

public:
    BendersCore(
        std::unique_ptr<ExecutionStrategy> executor,
        std::unique_ptr<BatchingStrategy> batcher,
        std::unique_ptr<OuterLoopStrategy> outer_loop,
        const BendersBaseOptions& options,
        Logger logger,
        std::shared_ptr<Output::OutputWriter> writer,
        std::shared_ptr<MathLoggerDriver> mathLoggerDriver
    ) : BendersBase(options, logger, writer, mathLoggerDriver),
        executor_(std::move(executor)),
        batcher_(std::move(batcher)),
        outer_loop_(std::move(outer_loop)) {
    }

    void launch() override {
        // 1. Pre-launch setup
        executor_->InitializeProblems();
        batcher_->PreLaunchSetup(coupling_map_, batch_size_);
        outer_loop_->BeforeLaunch();

        // 2. Main loop
        int iteration = 0;
        while (iteration++ < max_iterations_) {
            Run();
            
            // 3. Check outer loop
            if (!outer_loop_->CheckAndUpdate()) {
                break;
            }
        }

        // 4. Post-launch cleanup
        outer_loop_->AfterLaunch();
        executor_->Cleanup();
    }

    void Run() override {
        // Main Benders iteration
        for (auto& batch : GetBatches()) {
            SubProblemDataMap batch_cuts;
            
            // Solve batch subproblems
            for (auto& [name, subproblem] : batch) {
                auto cut = executor_->SolveSubproblem(subproblem, name);
                batch_cuts[name] = cut;
            }
            
            // Process batch
            batcher_->ProcessBatch(batch_cuts, ...);
            
            // Check batch-specific stopping
            if (batcher_->ShouldStop()) {
                break;
            }
        }
        
        // Gather and build cuts
        executor_->GatherCuts(local_cuts, all_cuts);
        master_build_cuts(all_cuts);
        
        // Update convergence
        ComputeConvergenceCriterion();
    }

    // Backward compat: old methods still work
    void InitializeProblems() override {
        executor_->InitializeProblems();
    }

    void free() override {
        executor_->Cleanup();
    }
};
```

---

### 6.2 Extract Common Logic from Old Implementations (0.5h)

**Checklist**:
- [ ] Identify common code in BendersMpi + BendersSequential
- [ ] Move to BendersCore::CommonMethod()
- [ ] Call from both old classes (backward compat)
- [ ] Verify duplication reduced

---

### 6.3 Maintain Backward Compatibility (0.5h)

**Goal**: Existing code using BendersBase still works

**Strategy**:
1. BendersBase interface remains unchanged
2. BendersCore implements BendersBase
3. Old classes (BendersMpi, BendersSequential) kept as deprecated wrappers (optional)

```cpp
// Backward compatibility
class BendersMpi : public BendersCore {
public:
    BendersMpi(...options...)
        : BendersCore(
            std::make_unique<ParallelMpiExecutor>(...),
            std::make_unique<NoBatchingStrategy>(),
            std::make_unique<NoOuterLoopStrategy>(),
            ...
        ) {}
};
```

---

### 6.4 Create PR#6

**Title**: `[CORE] Benders Refactor - BendersCore Implementation`

**Checklist**:
- [ ] BendersCore class fully implemented
- [ ] Strategies composed correctly
- [ ] Backward compatibility maintained
- [ ] All tests passing
- [ ] Code review: 2+ approvals

---

### Deliverable for Phase 6
- ✅ BendersCore consolidation complete
- ✅ All strategies integrated
- ✅ Backward compatibility maintained
- ✅ PR#6 merged

---

## PHASE 7: Factory Refactoring (Week 8, 2 hours)

### 7.1 Refactor BendersFactory (1h)

**Before**:
```cpp
switch (method_) {
    case BENDERS: benders = new BendersMpi(...); break;
    case BENDERS_OUTERLOOP: benders = new BendersMpiOuterLoop(...); break;
    case BENDERS_BY_BATCH: benders = new BendersByBatch(...); break;
    case BENDERS_BY_BATCH_OUTERLOOP: benders = new BendersByBatch(...); break;
}
```

**After**:
```cpp
BendersBase* BendersFactory::CreateBenders(
    ExecutionMode exec_mode, BatchMode batch_mode, OuterLoopMode ol_mode
) {
    auto executor = CreateExecutionStrategy(exec_mode);
    auto batcher = CreateBatchingStrategy(batch_mode);
    auto outer_loop = CreateOuterLoopStrategy(ol_mode);
    
    return new BendersCore(
        std::move(executor),
        std::move(batcher),
        std::move(outer_loop),
        ...
    );
}

std::unique_ptr<ExecutionStrategy> CreateExecutionStrategy(ExecutionMode mode) {
    switch (mode) {
        case ExecutionMode::MPI:
            return std::make_unique<ParallelMpiExecutor>(...);
        case ExecutionMode::SEQUENTIAL:
            return std::make_unique<SequentialExecutor>(...);
        default:
            throw std::invalid_argument("Unknown execution mode");
    }
}

// Similar for batcher and outer_loop
```

---

### 7.2 Update BendersApp::Run() (0.5h)

**Before**:
```cpp
auto benders = factory.ConfigureBenders(...);
// Runtime decision on OuterLoop
if (options_.DO_OUTER_LOOP) {
    outer_loop_benders = new OuterLoopBenders(benders, ...);
    outer_loop_benders->Run();
} else {
    benders->launch();
}
```

**After**:
```cpp
auto benders = factory.CreateBenders(
    exec_mode,
    batch_mode,
    options_.DO_OUTER_LOOP ? OuterLoopMode::ACTIVE : OuterLoopMode::NONE
);
benders->launch();  // Unified
```

---

### 7.3 Create PR#7

**Title**: `[REFACTOR] Benders Factory Refactoring`

**Checklist**:
- [ ] Factory uses strategies instead of switch/case
- [ ] OuterLoop creation in factory (not BendersApp)
- [ ] All 4 variant combinations creatable
- [ ] Tests passing
- [ ] Code review: 1 approval

---

### Deliverable for Phase 7
- ✅ Factory refactored
- ✅ No switch/case explosion
- ✅ PR#7 merged

---

## PHASE 8: MathLogger Adaptation (Week 9, 3 hours)

### 8.1 Analyze Current MathLogger Specializations (1h)

**Current State**:
```
MathLoggerBase
├─ MathLoggerBendersByBatch (extends MathLoggerBase)
├─ MathLoggerBaseExternalLoop (extends MathLoggerBase)
└─ MathLoggerBendersByBatchExternalLoop (extends MathLoggerBase)
```

**New State**:
```
MathLoggerBase (unified)
├─ Detects batching mode internally
├─ Detects outer loop internally
└─ No inheritance hierarchy needed
```

---

### 8.2 Refactor MathLogger (1.5h)

**Strategy**:
1. Keep MathLoggerBase as unified class
2. Add mode detection: IsBatching(), HasOuterLoop()
3. Log differently based on modes (internal if/else)
4. Remove specialized subclasses

```cpp
class MathLogger : public MathLoggerDriver {
private:
    bool is_batching_;
    bool has_outer_loop_;

public:
    void Print(const LogData& data) override {
        if (is_batching_) {
            PrintBatchingInfo(data);
        }
        if (has_outer_loop_) {
            PrintOuterLoopInfo(data);
        }
        PrintCommon(data);
    }
};
```

---

### 8.3 Test MathLogger Output (0.5h)

- [ ] Verify logging output unchanged vs old specialized classes
- [ ] Check batching log entries present when batching enabled
- [ ] Check outer loop log entries present when outer loop enabled
- [ ] Diff output files vs baseline

---

### 8.4 Create PR#8

**Title**: `[REFACTOR] Benders MathLogger Unification`

**Checklist**:
- [ ] MathLogger unified (no specializations)
- [ ] Output identical to old versions
- [ ] Logging tests passing
- [ ] Code review: 1 approval

---

### Deliverable for Phase 8
- ✅ MathLogger unified
- ✅ Logging output verified
- ✅ PR#8 merged

---

## PHASE 9: Tests, Validation, Regression (Week 10+, 10 hours)

### 9.1 Unit Tests for Each Strategy (3h)

**ParallelMpiExecutor Tests**:
- [ ] test_parallel_mpi_executor_initialize()
- [ ] test_parallel_mpi_executor_solve_subproblem()
- [ ] test_parallel_mpi_executor_gather_cuts()
- [ ] test_parallel_mpi_executor_broadcast_indices()

**SequentialExecutor Tests**:
- [ ] test_sequential_executor_initialize()
- [ ] test_sequential_executor_solve_subproblem()
- [ ] test_sequential_executor_gather_cuts_local()

**BatchingStrategy Tests**:
- [ ] test_no_batching_strategy()
- [ ] test_by_batch_strategy_multiple_batches()

**OuterLoopStrategy Tests**:
- [ ] test_no_outer_loop_strategy_noop()
- [ ] test_outer_loop_wrapper_iterations()

---

### 9.2 Integration Tests (2h)

**Variant Combinations** (ensure all work):
- [ ] MPI + NoBatch + NoOuterLoop
- [ ] MPI + ByBatch + NoOuterLoop
- [ ] MPI + NoBatch + OuterLoop
- [ ] MPI + ByBatch + OuterLoop
- [ ] Sequential + NoBatch + NoOuterLoop
- [ ] Sequential + ByBatch + NoOuterLoop (NEW)
- [ ] Sequential + NoBatch + OuterLoop (NEW)
- [ ] Sequential + ByBatch + OuterLoop (NEW)

**Test Template**:
```cpp
TEST_CASE("MPI + ByBatch + OuterLoop", "[integration]") {
    auto executor = std::make_unique<ParallelMpiExecutor>(...);
    auto batcher = std::make_unique<ByBatchStrategy>(...);
    auto outer_loop = std::make_unique<OuterLoopWrapper>(...);
    
    BendersCore benders(
        std::move(executor),
        std::move(batcher),
        std::move(outer_loop),
        ...
    );
    
    benders.launch();
    
    auto result = benders.GetBestIterationData();
    REQUIRE(result.value_obj >= baseline_obj - tolerance);
}
```

---

### 9.3 Regression Testing (2h)

**Baseline Comparison**:
```bash
# Generate baseline before refactoring
for variant in BENDERS BENDERS_BY_BATCH BENDERS_OUTERLOOP BENDERS_BY_BATCH_OUTERLOOP; do
    old_benders --method=$variant > baseline_$variant.log 2>&1
done

# Run refactored version
for variant in BENDERS BENDERS_BY_BATCH BENDERS_OUTERLOOP BENDERS_BY_BATCH_OUTERLOOP; do
    new_benders --method=$variant > refactored_$variant.log 2>&1
done

# Compare
for variant in BENDERS BENDERS_BY_BATCH BENDERS_OUTERLOOP BENDERS_BY_BATCH_OUTERLOOP; do
    obj_diff=$(diff baseline_$variant.log refactored_$variant.log | grep "obj" | head -1)
    echo "Variant: $variant, Diff: $obj_diff"
done
```

**Acceptance Criteria**:
- Objective value: ±1e-5 (floating point tolerance)
- Number of iterations: ±2 (convergence may vary slightly)
- Walltime: ±5%

---

### 9.4 Performance Profiling (1h)

**Profile MPI Communication Overhead**:
```bash
# Profile before refactoring
perf record old_benders --method=BENDERS_BY_BATCH
perf report > perf_old.txt

# Profile after refactoring
perf record new_benders --method=BENDERS_BY_BATCH
perf report > perf_new.txt

# Compare hotspots
diff perf_old.txt perf_new.txt
```

**Expected Overhead**:
- Virtual call overhead: < 1% (acceptable)
- MPI communication dominates (~70% of time)
- Subproblem solve dominates (~20% of time)

---

### 9.5 Documentation Update (1.5h)

**Files to Update**:
- [ ] `docs/architecture/benders-architecture.md` (new)
- [ ] `BENDERS_ARCHITECTURE_ANALYSIS.md` (update status)
- [ ] Add Doxygen comments to strategies
- [ ] Update code comments in BendersCore
- [ ] Migration guide for consumers

**Documentation Template**:
```markdown
# Benders Architecture (Post-Refactoring)

## Overview
Benders algorithm now uses Strategy Pattern for clean composition.

## Structure
- BendersCore: orchestrates algorithm
- ExecutionStrategy: MPI vs Sequential
- BatchingStrategy: None vs ByBatch
- OuterLoopStrategy: None vs Active

## Examples

### Create MPI variant
```cpp
auto benders = factory.CreateBenders(
    ExecutionMode::MPI,
    BatchMode::NONE,
    OuterLoopMode::NONE
);
benders->launch();
```

### Create Sequential + ByBatch + OuterLoop (new feature!)
```cpp
auto benders = factory.CreateBenders(
    ExecutionMode::SEQUENTIAL,
    BatchMode::BY_BATCH,
    OuterLoopMode::ACTIVE
);
benders->launch();
```

## Migration Guide
...
```

---

### 9.6 Code Review & Feedback Incorporation (1h)

**Review Checklist**:
- [ ] All PRs reviewed by 2+ engineers
- [ ] Architecture clear to team
- [ ] Tests cover edge cases
- [ ] Documentation complete
- [ ] Feedback incorporated

---

### 9.7 Create PR#9

**Title**: `[TEST] Benders Refactor - Tests & Documentation`

**Checklist**:
- [ ] Unit tests passing
- [ ] Integration tests passing (all 8 combinations)
- [ ] Regression tests passing (±1e-5 tolerance)
- [ ] Performance profiling done (< 5% overhead acceptable)
- [ ] Documentation updated
- [ ] Code review: 2+ approvals

---

### Deliverable for Phase 9
- ✅ Comprehensive test coverage
- ✅ All variant combinations validated
- ✅ Regression testing complete
- ✅ Performance verified
- ✅ Documentation updated
- ✅ PR#9 merged

---

## Post-Implementation

### 10.1 Monitor Production (Week 11-12, 2 weeks)

- [ ] Deploy to staging
- [ ] Run extended test suite (2+ days)
- [ ] Monitor for regressions
- [ ] Gather team feedback
- [ ] Hotfix if needed

---

### 10.2 Cleanup Old Code (Week 13, 1-2 days)

- [ ] Remove old BendersMpi (if not used externally)
- [ ] Remove old BendersSequential
- [ ] Remove old OuterLoopBenders (absorbed into OuterLoopStrategy)
- [ ] Clean up deprecated factory methods

---

### 10.3 Final Metrics

**Before**:
- BendersBase: 363 lines
- BendersMpi: 400 lines code
- BendersSequential: 400 lines code (duplicate)
- BendersByBatch: 500 lines code
- Total: ~2000 lines (with 30% duplication)

**After**:
- BendersBase: 363 lines (interface only)
- BendersCore: 300 lines (orchestration)
- ParallelMpiExecutor: 250 lines
- SequentialExecutor: 250 lines (no duplication!)
- NoBatchingStrategy: 50 lines
- ByBatchStrategy: 300 lines
- NoOuterLoopStrategy: 20 lines
- OuterLoopWrapper: 150 lines
- Total: ~1680 lines (0% duplication)

**Achievements**:
- ✅ ~320 lines removed (16% reduction)
- ✅ 100% duplication elimination
- ✅ 4 new combinations enabled (Sequential+OuterLoop, etc.)
- ✅ Factory scalability improved
- ✅ Maintenance burden reduced 40%

---

## Risk Mitigation Strategies

| Risk | Mitigation |
|------|-----------|
| **MPI regression** | Keep old code until 2-week monitoring complete |
| **Sequential path untested** | Add comprehensive tests Phase 3 |
| **OuterLoop bugs** | Dedicated PR review by OuterLoop expert |
| **Performance overhead** | Profile before/after, accept < 5% |
| **Team adoption** | Architecture talk + good documentation |
| **Backward compat** | Adapter classes if needed, migration guide |

---

## Success Criteria (Final)

- ✅ All 8 variant combinations working
- ✅ MPI/Sequential tests passing (regression < 1e-5)
- ✅ Performance overhead < 5%
- ✅ Zero code duplication
- ✅ OuterLoop works with Sequential
- ✅ Factory scalable (< 50 lines, lookup-table based)
- ✅ Documentation complete
- ✅ Team comfortable maintaining code

---

## Timeline Summary

```
WEEK 1:   Phase 1 - Design (5h)
WEEK 2-3: Phase 2 - ParallelMpiExecutor (8h)
WEEK 4:   Phase 3 - SequentialExecutor (5h)
WEEK 5:   Phase 4 - BatchingStrategy (3h)
WEEK 6:   Phase 5 - OuterLoopStrategy (2h)
WEEK 7:   Phase 6 - BendersCore (4h)
WEEK 8:   Phase 7 - Factory (2h)
WEEK 9:   Phase 8 - MathLogger (3h)
WEEK 10+: Phase 9 - Tests & Validation (10h)

TOTAL: 6 weeks (42 hours → 39 hours developer time estimate)
```

---

**Document**: Benders Architecture Refactoring - Implementation Checklist  
**Status**: PLANNING  
**Last Updated**: 2026-02-16  
**Target Start**: TBD (after team alignment)

