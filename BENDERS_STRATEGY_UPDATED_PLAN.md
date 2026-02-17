# Benders Strategy Pattern Refactoring - Updated Plan

## Executive Summary

**Objective**: Migrate Benders engine from inheritance-based design to Strategy pattern composition to:
- Remove code duplication between MPI and sequential variants
- Separate responsibilities (execution, batching, outer-loop)
- Make implementations combinable and extensible
- Stabilize MPI path first

**Status**: Phase 1 complete (interfaces & basic adapters). Ready for Phase 2 (concrete strategy implementations).

---

## Architecture Overview

### Current State (Inheritance-based)
```
BendersBase (abstract)
  ├── BendersSequential
  ├── BendersMPI
  ├── BendersByBatch
  └── OuterLoopBenders
```
**Problem**: Duplication, tight coupling, difficult to combine features (e.g., MPI + batching + outer-loop)

### Target State (Strategy pattern)
```
BendersCore (orchestrator)
  ├── IExecutionStrategy (Sequential | ParallelMpi)
  ├── IBatchingStrategy (NoBatching | ByBatch)
  └── IOuterLoopStrategy (NoOuterLoop | OuterLoop)
```
**Benefits**: Composition over inheritance, single responsibility, easy to test and extend

---

## Implementation Phases

### Phase 1: Infrastructure ✅ COMPLETE
**Goal**: Define interfaces and basic adapters

**Completed**:
- ✅ Created strategy interfaces:
  - `IBendersCore` - Main orchestrator interface
  - `IExecutionStrategy` - Execution strategy (Sequential/MPI)
  - `IBatchingStrategy` - Batching strategy
  - `IOuterLoopStrategy` - Outer loop strategy
- ✅ Created `BendersBaseAdapter` (IBendersCore wrapper for legacy BendersBase)
- ✅ Added adapter smoke test
- ✅ Build system integration (ENABLE_BENDERS_STRATEGY CMake option)

**Files**:
- `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/*.h`
- `src/cpp/benders/adapters/include/antares-xpansion/benders/adapters/BendersBaseAdapter.h`
- `src/cpp/benders/strategy/tests/adapter_smoke_test.cpp`

---

### Phase 2: Concrete Strategy Implementations 🚧 IN PROGRESS

#### 2.1 ExecutionStrategy Implementations ✅ COMPLETE
**Goal**: Wrap existing Sequential/MPI implementations as strategies

**Tasks**:
- [x] **SequentialExecutionStrategy** ✅
  - Create class wrapping `BendersSequential` into `IExecutionStrategy`
  - Delegate `launch()`, `InitializeProblems()`, `Run()` to wrapped instance
  - Add unit tests (6 test cases)
  - **Files**: `src/cpp/benders/strategy/include/.../SequentialExecutionStrategy.h`
  - **Status**: Complete in PR #2

- [x] **ParallelMpiExecutionStrategy** ✅
  - Create class wrapping `BendersMPI` into `IExecutionStrategy`
  - Handle MPI communicator injection (constructor parameter)
  - Delegate execution methods to wrapped BendersMPI instance
  - Add MPI-aware tests (7 test cases)
  - **Files**: `src/cpp/benders/strategy/include/.../ParallelMpiExecutionStrategy.h`
  - **Status**: Complete in PR #3

**Acceptance Criteria**: ✅ ALL MET
- ✅ Strategies compile and link successfully
- ✅ Unit tests complete for Sequential variant (6 tests)
- ✅ Unit tests complete for MPI variant (7 tests)
- ✅ MPI communicator handling validated

#### 2.2 BatchingStrategy Implementations ✅ COMPLETE
**Goal**: Implement batching variants

**Tasks**:
- [x] **NoBatchingStrategy** ✅ (passthrough, no batching logic)
  - 5 comprehensive unit tests
  - Header-only, minimal dependencies
  - **Status**: Complete in PR #4
- [x] **ByBatchStrategy** ✅ (wrap BendersByBatch logic)
  - 6 comprehensive unit tests
  - Wraps BendersByBatch implementation
  - **Status**: Complete in PR #4

**Files**: `src/cpp/benders/strategy/include/.../NoBatchingStrategy.h`, `ByBatchStrategy.h`

#### 2.3 OuterLoopStrategy Implementations ✅ COMPLETE
**Goal**: Implement outer-loop variants

**Tasks**:
- [x] **NoOuterLoopStrategy** ✅ (passthrough)
  - 11 comprehensive unit tests
  - Header-only, minimal dependencies
  - **Status**: Complete in PR #5
- [x] **OuterLoopAdapter** ✅ (wrap OuterLoop logic)
  - 10 comprehensive unit tests
  - Wraps Outerloop::OuterLoop implementation
  - **Status**: Complete in PR #5

**Files**: `src/cpp/benders/strategy/include/.../NoOuterLoopStrategy.h`, `OuterLoopAdapter.h`

**Phase 2 Summary**: ✅ **ALL COMPLETE**
- Total test count: 45 tests across all strategies
- All strategies follow consistent delegation patterns
- Zero code review issues
- All acceptance criteria met

---

### Phase 3: BendersCore Orchestration ✅ COMPLETE
**Goal**: Implement the main orchestrator that composes strategies

**Tasks**:
- [x] Create `BendersCore` class implementing `IBendersCore` ✅
  - Constructor accepts unique_ptrs to three strategies
  - Orchestrates `launch()`: init outer loop → init batching → init execution → run → update batching
  - Delegates methods (`BendersName`, `execution_time`, `InitializeProblems`) to appropriate strategies
  - Null-safe implementation (strategies can be nullptr)
- [x] Add comprehensive unit tests ✅
  - 13 test cases covering orchestration, delegation, null safety
  - Mock strategies for isolated testing
- [x] Integration tests for combinations ✅
  - Execution only
  - Execution + Batching
  - Execution + OuterLoop
  - Full configuration (all 3 strategies)

**Files**: 
- `src/cpp/benders/strategy/include/.../BendersCore.h` ✅
- `src/cpp/benders/strategy/tests/BendersCore_test.cpp` ✅

**Status**: Complete in PR #6
- Code review: 2 issues found and fixed
- Security scan: clean
- Total tests: 58 (45 strategies + 13 orchestration)

---

### Phase 4: Factory Refactoring ✅ COMPLETE
**Goal**: Update BendersFactory to build strategies instead of concrete classes

**Tasks**:
- [x] Refactor `BendersFactory` to support strategy creation ✅
  - Add `PrepareForExecutionWithStrategies()` method
  - Add `ConfigureBendersWithStrategies()` internal method
  - Build ExecutionStrategy (ParallelMpiExecutionStrategy)
  - Build BatchingStrategy (ByBatch or NoBatching based on BATCH_SIZE)
  - Build OuterLoopStrategy (OuterLoop or NoOuterLoop based on flag)
  - Return unique_ptr<IBendersCore> wrapping BendersCore with composed strategies
- [x] Maintain backward compatibility ✅
  - Feature flag approach (ENABLE_BENDERS_STRATEGY)
  - Original methods unchanged
  - New methods added alongside
- [x] Documentation ✅
  - FACTORY_STRATEGY_TESTING.md created
  - Strategy selection logic documented
  - Testing strategy outlined

**Files**: 
- `src/cpp/benders/factories/BendersFactory.{h,cpp}` ✅
- `FACTORY_STRATEGY_TESTING.md` ✅

**Status**: Complete in PR #7
- Code compiles with and without ENABLE_BENDERS_STRATEGY
- Factory can create strategy-based BendersCore
- All 4 BENDERSMETHOD variants supported
- Backward compatible with existing code

**Notes**:
- Some IBendersCore methods need extension (set_input_map, etc.)
- Sequential strategy not yet wired (only ParallelMPI)
- Comprehensive integration tests recommended for production

---

### Phase 5: Cleanup & Validation 🚧 NEXT
**Goal**: Remove duplication, unify implementations, validate performance

**Tasks**:
- [ ] Extend IBendersCore interface if needed
  - Add set_input_map, setCriterionComputationInputs
  - Add solver log configuration
- [ ] Add Sequential execution option to factory
- [ ] Unify MathLogger implementations (remove duplicates in Sequential/MPI)
- [ ] Remove obsolete code (once all paths migrate to strategy)
- [ ] Full integration testing with real study data
- [ ] Performance validation:
  - Run benchmark studies
  - Compare wall time and objective values vs. baseline
  - **Acceptance**: delta obj < tolerance, wall time < 5% regression
- [ ] Update documentation (architecture docs, developer guide)

**Estimate**: 2-3 days

---

## PR Sequence (Incremental Delivery)

### PR #1: Stabilize Interfaces & Adapters
**Scope**: Phase 1 completion verification
- Strategy interface headers
- BendersBaseAdapter with [[nodiscard]]
- adapter_smoke_test
- CMake integration (ENABLE_BENDERS_STRATEGY flag)

**Branch**: `feature/benders-strategy-refactor` (base for all PRs)  
**Target**: `develop` (or `main`)  
**Estimate**: 0.5 day (review & merge)

---

### PR #2: SequentialExecutionStrategy
**Scope**: Sequential executor implementation
- SequentialExecutionStrategy.h/cpp
- Unit tests
- Documentation

**Dependencies**: PR #1  
**Estimate**: 1 day

---

### PR #3: ParallelMpiExecutionStrategy
**Scope**: MPI executor implementation
- ParallelMpiExecutionStrategy.h/cpp
- MPI smoke tests
- Handle communicator injection carefully

**Dependencies**: PR #2  
**Estimate**: 1.5 days

---

### PR #4: Batching & OuterLoop Strategies
**Scope**: Remaining strategy implementations
- NoBatchingStrategy, ByBatchStrategy
- NoOuterLoopStrategy, OuterLoopWrapper
- Unit tests for each

**Dependencies**: PR #3  
**Estimate**: 1.5 days

---

### PR #5: BendersCore Orchestration
**Scope**: Main orchestrator
- BendersCore.h/cpp
- Orchestration logic
- Integration tests for combinations

**Dependencies**: PR #4  
**Estimate**: 2 days

---

### PR #6: Factory Refactoring
**Scope**: Update factory to build strategies
- BendersFactory changes
- Incremental migration path
- Backward compatibility

**Dependencies**: PR #5  
**Estimate**: 1.5 days

---

### PR #7: Cleanup & Finalization
**Scope**: Remove duplication, validate performance
- Unify MathLogger
- Remove obsolete code
- Performance benchmarks
- Documentation updates

**Dependencies**: PR #6  
**Estimate**: 2 days

**Total Timeline**: ~10-12 days (assuming sequential PRs with review cycles)

---

## Build & Test Commands

### Configure (if not already done)
```bash
cmake -B _build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux-release \
  -DBUILD_TESTING=ON \
  -DENABLE_BENDERS_STRATEGY=ON
```

### Build
```bash
# Full build
cmake --build _build --target all -j 6

# Specific target
cmake --build _build --target adapter_smoke_test -j 6
```

### Test
```bash
cd _build

# Run all tests
ctest -C Release --output-on-failure

# Run strategy tests only
ctest -C Release --output-on-failure -R adapter

# List tests
ctest -N
```

### On remote "jm" (per problem statement)
```bash
/home/jmarechal/miniconda3/bin/cmake --build /home/jmarechal/CLionProjects/build_xpansion_relwithdebinfo --target all -j 6
```

---

## Key Design Decisions

### 1. Composition over Inheritance
**Rationale**: Strategies are composed in BendersCore rather than using inheritance. This allows runtime combination of features (e.g., MPI + batching + outer-loop) without exponential class explosion.

### 2. Non-owning vs Owning Adapters
- `BendersBaseAdapter`: Non-owning (uses reference), for wrapping existing instances
- Strategy adapters: Owning (use unique_ptr or by-value), for new orchestration

### 3. Incremental Migration
- ENABLE_BENDERS_STRATEGY flag allows opt-in during development
- Factory supports both legacy and strategy paths during transition
- Existing tests continue to run against legacy implementation

### 4. MPI Communicator Injection
- ParallelMpiExecutionStrategy accepts MPI_Comm as constructor parameter
- Avoids global MPI state, enables testing with custom communicators

---

## Risk Mitigation

### Performance Regression
**Risk**: Strategy indirection adds overhead  
**Mitigation**: 
- Inline small delegation methods
- Use unique_ptr (zero-cost abstraction) rather than virtual calls where possible
- Benchmark early and often
- Accept <5% wall time increase as acceptable trade-off for maintainability

### MPI Stability
**Risk**: MPI path is complex, easy to break  
**Mitigation**:
- Start with sequential implementation (simpler)
- Add MPI-specific smoke tests before full integration
- Test with different MPI ranks and configurations

### Merge Conflicts
**Risk**: Long-lived feature branch diverges from main  
**Mitigation**:
- Incremental PRs (merge frequently)
- Keep main/develop up-to-date via regular rebases
- Small, focused PRs reduce review burden

---

## Success Criteria

### Functional
- [ ] All existing tests pass with new strategy implementation
- [ ] New strategy combinations work (MPI+batch+outer)
- [ ] Factory produces correct strategy instances based on options

### Non-Functional
- [ ] Code duplication reduced by >50% (measured via cloc or similar)
- [ ] MathLogger unified to single implementation
- [ ] Performance delta: objective < tolerance, wall time < 5% increase
- [ ] Code coverage for strategy code >80%

### Process
- [ ] All PRs reviewed and merged
- [ ] CI green for all commits
- [ ] Documentation updated (architecture, developer guide)

---

## Next Immediate Steps

1. **Create BENDERS_STRATEGY_UPDATED_PLAN.md** ✅ (this file)
2. **Implement SequentialExecutionStrategy** (PR #2)
   - Create header/source files
   - Wrap BendersSequential
   - Add unit tests
   - Build & verify
3. **Implement ParallelMpiExecutionStrategy** (PR #3)
4. **Continue with Phase 2 remaining tasks...**

---

## Notes & Context

- **Session context**: User requested to "proceed with plan on remote jm"
- **Build command**: `/home/jmarechal/miniconda3/bin/cmake --build /home/jmarechal/CLionProjects/build_xpansion_relwithdebinfo --target all -j 6`
- **Clang-tidy fix applied**: Added [[nodiscard]] to BendersBaseAdapter value-returning methods
- **Design pattern**: Strategy pattern for separating algorithm variants (execution/batching/outer-loop)

---

## References

- Strategy Pattern: https://refactoring.guru/design-patterns/strategy
- IBendersCore interface: `src/cpp/benders/strategy/include/antares-xpansion/benders/strategy/IBendersCore.h`
- Existing implementations:
  - BendersSequential: `src/cpp/benders/benders_sequential/`
  - BendersMPI: `src/cpp/benders/benders_mpi/`
  - BendersByBatch: `src/cpp/benders/benders_by_batch/`
  - OuterLoopBenders: `src/cpp/benders/outer_loop/`
