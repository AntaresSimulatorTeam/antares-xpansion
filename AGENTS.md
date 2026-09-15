# AGENTS.md

Quick reference for working on Antares Xpansion. See [docs/agents/](docs/agents/) for detailed guides.

## Project

Investment optimization solver for Antares power system studies (C++/Python).

## Build

```bash
cmake --preset vcpkg
cmake --build build
```

## Test

```bash
cmake --preset vcpkg -DBUILD_TESTING=ON && cmake --build build
ctest --test-dir build -L unit                    # unit tests (C++ and Python)
ctest --test-dir build -L unit -E unit_launcher   # C++ unit tests only
```

Select tests by label, not by name pattern — C++ unit-test names are
inconsistent, so `-R '^unit_'` silently skips about half of them.
See the [Testing Guide](docs/agents/testing.md) for end-to-end and BDD suites.

Without preset, configure with the vcpkg toolchain explicitly (Linux preset equivalent):
`cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux-release`

## Key Paths

- `src/cpp/` - C++ source
- `src/python/` - Python source
- `tests/cpp/` - C++ tests (Google Test)
- `tests/python/` - Python tests (pytest)
- `tests/end_to_end/` - End-to-end tests, including the cucumber/behave BDD suite
- `docs/` - Documentation

## Format Code

```bash
clang-format -i -style=file <file>.cpp
```

## More

- [C++ Conventions](docs/agents/cpp-conventions.md)
- [Python Conventions](docs/agents/python-conventions.md)
- [Testing Guide](docs/agents/testing.md)
