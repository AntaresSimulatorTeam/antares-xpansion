# C++ Conventions

This document covers C++ coding conventions for Antares-Xpansion.

## Language Standard

- C++20 is required
- GCC >= 11 (required for Intel TBB compatibility)

## Code Style

### Formatting

Use `.clang-format` with Chromium-based style:

```bash
clang-format -i -style=file <file>.cpp
```

Disable formatting for specific sections:

```cpp
// clang-format off
// code here will not be reformatted
// clang-format on
```

### Style Rules

- **No comments** unless explaining complex algorithms
- Use `nullptr` instead of `NULL`
- Use `std::array` or `std::vector` instead of C-style arrays
- Use `const` wherever possible
- Use `[[nodiscard]]` attribute for functions that must return a value
- Use `override` keyword when overriding virtual functions

### Naming

- Classes: `PascalCase` (e.g., `BendersSolver`)
- Public member functions: `PascalCase` (e.g., `InitializeSolver`)
- Private member functions / helper functions: `snake_case` (e.g., `initialize_solver`)
- Variables: `snake_case` (e.g., `iteration_count`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_ITERATIONS`)
- Private members: `snake_case_` (trailing underscore)

### Include Order

1. Project headers (quoted)
2. Standard library
3. External libraries (angled)

```cpp
#include "benders/benders_core/BendersOptions.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

#include <vector>
#include <memory>

#include <boost/mpi.hpp>
```

## Project Structure

```
src/cpp/
├── benders/                  # Benders decomposition algorithm
├── core/                     # Core shared utilities
├── exe/                      # Executable entry points
├── full_run/                 # Full run orchestration
├── helpers/                  # Shared helpers
├── lpnamer/                  # Generation of investment problems from simulation ones
├── merge_weights_trajectory/ # Trajectory weight merging
├── multisolver_interface/    # Solver abstraction layer
├── presolve/                 # Pre-solve logic
├── sensitivity/              # Sensitivity analysis
├── study-updater/            # Study updater
└── xpansion_interfaces/      # Public interfaces
```

### Header Organization

- Each class in its own header file
- Use include guards (or `#pragma once`)
- Group related functionality into subdirectories

## Dependencies

### External Libraries

- **Boost**: MPI, serialization, program_options
- **Coin-OR**: Clp, Cbc, Cgl, CoinUtils, Osi
- **JSON**: jsoncpp
- **MPI**: OpenMPI
- **TBB**: Intel Threading Building Blocks

### Finding Dependencies

The project uses CMake with vcpkg. Most dependencies are handled automatically.

## Error Handling

- Use exceptions for unrecoverable errors
- Prefer `std::optional` for functions that may not return a value

## Performance

- Avoid unnecessary copies (use const references)
- Use move semantics where appropriate
- Consider lazy evaluation for expensive operations
- Profile before optimizing

## Logging

Use the project's logging infrastructure:

```cpp
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

// In class constructor
ILogger::Ptr logger_ = ...;
logger_->log_at_initialization("message");
```
