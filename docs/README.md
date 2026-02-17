# Benders Strategy Pattern Documentation

## Overview

This directory contains comprehensive documentation for the Benders Strategy Pattern refactoring in Antares-Xpansion.

## Documentation Index

### Architecture Decision Record (ADR)

**[ADR 0001: Benders Strategy Pattern](architecture/adr/0001-benders-strategy-pattern.md)**
- Why we adopted the Strategy pattern
- What problems it solves
- Implementation phases
- Consequences and trade-offs

### Architecture Documentation

**[Architecture Overview](architecture/benders-strategy-overview.md)**
- High-level architecture diagrams
- Component relationships
- Design principles
- Strategy combinations
- Execution flow

### Developer Guides

**[Developer Guide](developer-guide/benders-strategy-guide.md)**
- Quick start guide
- How to use the Strategy pattern
- Adding new strategies
- Common patterns and best practices
- Troubleshooting

**[Code Navigation Guide](developer-guide/code-navigation.md)**
- Directory structure
- Key files and their purposes
- Finding what you need
- Entry points for common tasks
- Quick reference

**[Testing Guide](developer-guide/testing-strategy-pattern.md)**
- Testing philosophy
- Test structure and patterns
- Running tests
- Adding new tests
- Mocking strategies

### API Reference

**[API Reference](api/benders-strategy-api.md)**
- Complete interface documentation
- All concrete strategies
- BendersCore orchestrator
- Factory API
- Code examples

## Quick Links

### I want to...

- **Understand why we refactored**: Read [ADR 0001](architecture/adr/0001-benders-strategy-pattern.md)
- **Learn the architecture**: Read [Architecture Overview](architecture/benders-strategy-overview.md)
- **Use the Strategy pattern**: Read [Developer Guide](developer-guide/benders-strategy-guide.md)
- **Find specific code**: Read [Code Navigation](developer-guide/code-navigation.md)
- **Write tests**: Read [Testing Guide](developer-guide/testing-strategy-pattern.md)
- **Look up an API**: Read [API Reference](api/benders-strategy-api.md)

### For New Developers

Start here:
1. [ADR 0001](architecture/adr/0001-benders-strategy-pattern.md) - Understand the "why"
2. [Architecture Overview](architecture/benders-strategy-overview.md) - Learn the "what"
3. [Developer Guide](developer-guide/benders-strategy-guide.md) - Learn the "how"

### For Experienced Developers

Quick references:
- [Code Navigation](developer-guide/code-navigation.md) - Find files quickly
- [API Reference](api/benders-strategy-api.md) - Look up interfaces
- [Testing Guide](developer-guide/testing-strategy-pattern.md) - Test patterns

## Architecture Summary

### The Pattern

The Benders engine uses the **Strategy Pattern** to separate three independent concerns:

```
BendersCore (orchestrator)
├── ExecutionStrategy (Sequential | MPI)
├── BatchingStrategy (NoBatching | ByBatch)
└── OuterLoopStrategy (NoOuterLoop | OuterLoop)
```

### Benefits

- ✅ **8 combinations available** (2×2×2)
- ✅ **Zero code duplication**
- ✅ **Clean separation of concerns**
- ✅ **Easy to extend** (add new strategies)
- ✅ **Better testability** (66 tests)
- ✅ **Backward compatible** (no breaking changes)

### Key Components

| Component | Purpose | Location |
|-----------|---------|----------|
| `IBendersCore` | Main interface | `strategy/include/.../IBendersCore.h` |
| `BendersCore` | Orchestrator | `strategy/include/.../BendersCore.h` |
| Execution Strategies | How problems are solved | `strategy/include/.../*ExecutionStrategy.h` |
| Batching Strategies | How problems are batched | `strategy/include/.../*BatchingStrategy.h` |
| OuterLoop Strategies | Outer-loop optimization | `strategy/include/.../*OuterLoopStrategy.h` |
| `BendersFactory` | Creates strategies | `factories/BendersFactory.*` |

## Usage Example

```cpp
#include "antares-xpansion/benders/factories/BendersFactory.h"

// Create factory
BendersFactory factory(dependencies);

// Get configured Benders instance
auto env = factory.PrepareForExecution(outer_loop);

// env->benders is IBendersCore* (BendersCore instance)
env->benders->set_input_map(coupling_map);
env->benders->launch();

// Get results
auto results = env->benders->GetBestIterationData();
```

That's it! The factory automatically selects appropriate strategies based on:
- MPI world size (Sequential vs. MPI)
- BENDERSMETHOD enum (batching and outer-loop)

## Project Status

✅ **100% Complete**

- All 5 phases finished
- All known limitations solved
- ENABLE_BENDERS_STRATEGY flag removed
- Strategy pattern is the default and only implementation
- 66 tests (all passing)
- Zero breaking changes
- Production ready

## Contributing

### Adding a New Strategy

1. Create strategy header implementing the appropriate interface
2. Add comprehensive unit tests
3. Update factory selection logic
4. Update documentation

See [Developer Guide](developer-guide/benders-strategy-guide.md#adding-new-strategies) for detailed instructions.

### Reporting Issues

If you find issues with the Strategy pattern implementation:
1. Check [Troubleshooting](developer-guide/benders-strategy-guide.md#troubleshooting)
2. Review existing tests in `src/cpp/benders/strategy/tests/`
3. Report issue with minimal reproduction case

## Related Documentation

- **Source code**: `src/cpp/benders/strategy/`
- **Tests**: `src/cpp/benders/strategy/tests/`
- **Factory**: `src/cpp/benders/factories/`
- **Legacy summary docs** (root directory):
  - `BENDERS_STRATEGY_FINAL_SUMMARY.md`
  - `KNOWN_LIMITATIONS_SOLVED.md`
  - `STRATEGY_MIGRATION_COMPLETE.md`

## Version History

- **2026-02-17**: Complete documentation added
- **2026-02-17**: ENABLE_BENDERS_STRATEGY flag removed
- **2026-02-17**: All known limitations solved
- **2026-02-17**: Phase 5 complete (final documentation)
- **2026-02-17**: Phase 4 complete (factory integration)
- **2026-02-17**: Phase 3 complete (BendersCore orchestration)
- **2026-02-17**: Phase 2 complete (all strategy implementations)
- **2026-02-17**: Phase 1 complete (infrastructure)

## License

Same as Antares-Xpansion project license.

---

**For questions or clarifications**, refer to the appropriate documentation section above or review the source code and tests.
