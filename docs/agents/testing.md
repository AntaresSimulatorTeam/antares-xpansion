# Testing Guide

This document covers testing patterns for Antares Xpansion.

## Building Tests

Enable tests during CMake configuration:

```bash
cmake -B build -S . -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running Tests

### All Tests

```bash
ctest --test-dir build
```

### Unit Tests Only (C++ and Python)

```bash
ctest --test-dir build -L unit
```

### C++ Unit Tests Only

```bash
ctest --test-dir build -R '^unit_' -E unit_launcher
```

### Python Unit Tests Only

```bash
ctest --test-dir build -R unit_launcher
```

### End-to-End Tests

```bash
ctest --test-dir build -L end_to_end
```

### By Duration Label

```bash
ctest --test-dir build -L short    # Fast tests
ctest --test-dir build -L medium    # Medium duration
ctest --test-dir build -L long      # Long running tests
```

### With Coverage

```bash
cmake -B build -S . -DBUILD_TESTING=ON -DCODE_COVERAGE=ON
cmake --build build
ctest --test-dir build
# Coverage report in build/coverage/
```

## Test Organization

### Test Names

| Test Pattern | Description |
|-------------|-------------|
| `unit_*` | C++ unit tests |
| `unit_launcher` | Python unit tests |
| `examples_*` | Example-based integration tests |
| `sequential`, `mpibenders` | Benders integration tests |

### Labels

- `unit` - Unit tests
- `end_to_end` - Integration tests
- `short`, `medium`, `long` - Duration categories
- `benders`, `lpnamer`, `bdd` - Functional categories

## C++ Tests (Google Test)

### Location

```
tests/cpp/
├── benders/
├── full_run/
├── helpers/
├── json_output_writer/
├── logger/
├── lp_namer/
├── merge_mps/
├── multisolver_interface/
├── outer_loop/
├── restart_benders/
├── sensitivity/
├── solvers_interface/
├── study_updater/
├── TestDoubles/
├── tests_utils/
└── zip_mps/
```

### Writing Tests

```cpp
#include <gtest/gtest.h>

class MyClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
};

TEST_F(MyClassTest, ShouldDoSomething) {
    MyClass obj;
    EXPECT_EQ(obj.compute(), expected_value);
}
```

### Assertions

- Use `EXPECT_*` for non-fatal failures (test continues)
- Use `ASSERT_*` for fatal failures (stops test)
- Prefer clear failure messages:

```cpp
EXPECT_EQ(actual, expected) << "Failed for input: " << input;
```

## Python Tests (pytest)

### Location

```
tests/python/
```

### Writing Tests

```python
import pytest

def test_load_candidates(study_path):
    reader = CandidatesReader(study_path)
    candidates = reader.load()
    assert len(candidates) == 5
    assert "candidate1" in candidates

@pytest.fixture
def study_path(tmp_path):
    # Create test study
    return tmp_path / "test_study"
```

### Markers

Available pytest markers (defined in `tests/python/pytest.ini` or conftest):
- `@pytest.mark.short_sequential`
- `@pytest.mark.short_mpi`
- `@pytest.mark.medium_*`
- `@pytest.mark.long_*`

Run specific markers:

```bash
pytest -m short_sequential tests/python/
```

## Cucumber / BDD Tests (behave)

End-to-end functional tests for the Benders solver live under
`tests/end_to_end/cucumber/`, using [behave](https://behave.readthedocs.io/)
(Gherkin).

### Location

```
tests/end_to_end/cucumber/features/
├── *.feature            # Scenario definitions
└── steps/
    ├── given.py          # Given steps (study path, batch size, cache problems level, ...)
    ├── when.py           # When steps (run benders / outer_loop / antares-xpansion)
    ├── then.py           # Then steps (assertions: cost, solution values, ...)
    └── environment.py    # before/after hooks
```

### Running

Always run `behave` from `tests/end_to_end/` (not from inside `cucumber/`) —
the `Given the study path is "..."` step resolves paths relative to that
directory:

```bash
cd tests/end_to_end
behave cucumber/features                                     # full suite
behave --tags '@short' cucumber/features/<file>.feature       # one file, tagged subset
```

This matches how CI invokes it (`.github/workflows/cucumber-tests/action.yml`),
which by default skips `@flaky` and `@noci`.

Requires a built `benders`/`outer_loop` executable and, for `MICRO_ITERATIONS`
studies, the `dummy_micro_iterations_plugin` target (see
`tests/cpp/plugins/CMakeLists.txt`, which copies it into the relevant
`data_test/*/plugin_inputs/` directories via a `POST_BUILD` step).

### Writing scenarios

Prefer a single `Scenario Outline` with an `Examples:` table over many
near-duplicate `Scenario`s when several scenarios share the same `Then`
assertions and only differ in a few `Given`/`When` parameters (study path,
cache level, batch size, proc count, ...) — see
`benders_memory_and_micro_iterations.feature` and `benders_aggreg_cuts.feature`.
Keep every `Examples` row's step sequence identical: `Scenario Outline` can't
conditionally add or remove a step per row, so use a parametrized step with a
harmless default value (e.g. `And the batch size is 0`) instead of omitting a
step for rows where it doesn't apply.

### Key `Given` steps (`steps/given.py`)

- `the study path is "<path>"` — copies the study into a temp dir; all later
  steps operate on that copy
- `the batch size is <n>` — sets `BATCH_SIZE` in `options.json`
- `the cache problems level is <n>` — sets `CACHE_PROBLEMS` in `options.json`
  (`NO_CACHE` = resident subproblems, `PER_SUB` = reload-from-disk with basis caching, `COMPACT` =
  compact skeleton representation — the last requires the study's `sub/`
  layout to actually be in the compact CSV format, not per-subproblem MPS)

## CI Requirements

- All tests must pass before merging
- Code coverage must not decrease significantly
- Run full test suite locally before pushing
