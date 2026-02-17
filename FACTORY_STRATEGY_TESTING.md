# BendersFactory Strategy Integration - Test Documentation

## Overview

This document describes how to test the strategy-based BendersFactory methods.

## Factory Methods

### Legacy Method (Always Available)
```cpp
auto PrepareForExecution(bool outer_loop) -> std::optional<BendersEnvironment>;
```
- Returns `BendersBase*` wrapped in environment
- Uses switch/case to create concrete implementations
- Original implementation, always available

### Strategy Method (ENABLE_BENDERS_STRATEGY Required)
```cpp
auto PrepareForExecutionWithStrategies(bool outer_loop) -> std::optional<StrategyBendersEnvironment>;
```
- Returns `IBendersCore*` wrapped in environment
- Composes ExecutionStrategy + BatchingStrategy + OuterLoopStrategy
- Only available when ENABLE_BENDERS_STRATEGY is defined

## Strategy Selection Logic

The factory maps BENDERSMETHOD enum to strategy combinations:

| Method | ExecutionStrategy | BatchingStrategy | OuterLoopStrategy |
|--------|------------------|------------------|-------------------|
| BENDERS | ParallelMpi | NoBatching | NoOuterLoop |
| BENDERS_OUTERLOOP | ParallelMpi | NoBatching | OuterLoop |
| BENDERS_BY_BATCH | ParallelMpi | ByBatch | NoOuterLoop |
| BENDERS_BY_BATCH_OUTERLOOP | ParallelMpi | ByBatch | OuterLoop |

## Method Deduction

BENDERSMETHOD is deduced from options:
- **batch_size** determines batching (0 or max → no batching)
- **outer_loop** flag determines outer loop
- Combinations create 4 different methods

## Testing Strategy

### Unit Tests (Ideal)
Would require:
- Mock SimulationOptions
- Mock MPI communicator
- Mock dependencies (logger, writer, etc.)
- Verify correct strategies are created for each method

### Integration Tests (Recommended)
- Use real options from test data
- Verify factory creates working implementations
- Test that strategy-based path produces same results as legacy

### Manual Verification
1. Build with `-DENABLE_BENDERS_STRATEGY=ON`
2. Run application with different options
3. Verify BendersCore is created with correct strategies
4. Compare results with legacy implementation

## Example Usage

```cpp
#ifdef ENABLE_BENDERS_STRATEGY
// Create factory
BendersFactory factory(options, &world, dependencies);

// Prepare with strategies
auto env = factory.PrepareForExecutionWithStrategies(/*outer_loop=*/true);

if (env)
{
    // env->benders is IBendersCore* (BendersCore instance)
    env->benders->launch();
    
    // Access timing
    double time = env->benders->execution_time();
    
    // Get name
    std::string name = env->benders->BendersName();
}
#endif
```

## Known Limitations

Current implementation has TODOs:
1. **set_input_map**: Not exposed by IBendersCore, currently set on underlying implementation
2. **setCriterionComputationInputs**: Not exposed by IBendersCore
3. **ConfigureSolverLog**: Expects BendersBase*, not available for IBendersCore
4. **Sequential Strategy**: Not yet implemented, only ParallelMpi available

## Future Work

1. Extend IBendersCore interface to expose needed methods
2. Add SequentialExecutionStrategy option
3. Implement comprehensive factory tests
4. Performance comparison between legacy and strategy paths
5. Migrate all code to use strategy-based factory

## Verification Checklist

- [ ] Factory compiles with ENABLE_BENDERS_STRATEGY=ON
- [ ] Factory compiles without ENABLE_BENDERS_STRATEGY (backward compatible)
- [ ] PrepareForExecutionWithStrategies creates BendersCore
- [ ] Correct strategies selected for each BENDERSMETHOD
- [ ] Strategy-based implementation can execute
- [ ] Results match legacy implementation
