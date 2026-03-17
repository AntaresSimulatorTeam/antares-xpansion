# AGENTS.md

Quick reference for working on Antares-Xpansion. See [docs/agents/](docs/agents/) for detailed guides.

## Project

Investment optimization solver for Antares power system studies (C++/Python).

## Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run tests: `cmake -B build -S . -DBUILD_TESTING=ON && ctest --test-dir build`

## Key Paths

- `src/cpp/` - C++ source
- `src/python/` - Python source
- `tests/cpp/` - C++ tests (Google Test)
- `tests/python/` - Python tests (pytest)
- `docs/` - Documentation

## Format Code

```bash
clang-format -i -style=file <file>.cpp
```

## More

- [C++ Conventions](docs/agents/cpp-conventions.md)
- [Python Conventions](docs/agents/python-conventions.md)
- [Testing Guide](docs/agents/testing.md)
