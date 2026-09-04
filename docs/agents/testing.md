# Testing Guide

Testing patterns for Antares Xpansion. Assumes a build configured with
`-DBUILD_TESTING=ON` (see [AGENTS.md](../../AGENTS.md)).

## Running Tests

```bash
ctest --test-dir build                       # everything
ctest --test-dir build -L unit               # all unit tests (C++ and Python)
ctest --test-dir build -L unit -E unit_launcher   # C++ unit tests only
ctest --test-dir build -R unit_launcher      # Python unit tests only
ctest --test-dir build -L end_to_end         # integration tests
```

Select by label rather than by name pattern: C++ unit-test *names* are
inconsistent (`unit_benders_sequential`, but also `helpers_test`,
`output_writer`, `test_multisolver`, `zip_mps_lib_tests`), so `-R '^unit_'`
silently skips about half of them. Every one of them carries the `unit` label.

### Labels

| Label | Meaning |
|---|---|
| `unit` | Unit tests (C++ and Python) |
| `end_to_end` | Integration tests |
| `short`, `medium`, `long` | Duration categories |
| `benders`, `lpnamer`, `bdd`, `gems`, `restart` | Functional categories |

Labels are assigned in each `tests/**/CMakeLists.txt` via
`set_property(TEST <name> PROPERTY LABELS ...)`.

### Coverage

```bash
cmake -B build -S . -DCODE_COVERAGE=ON   # implies BUILD_TESTING=ON
cmake --build build
ctest --test-dir build
cmake --build build --target code-coverage   # report in build/coverage/
```

## C++ Tests (Google Test)

Live in `tests/cpp/<component>/`, one CMake target per component. Standard
GoogleTest — prefer `EXPECT_*` over `ASSERT_*` unless a failure makes the rest
of the test meaningless, and attach context to non-obvious assertions:

```cpp
EXPECT_EQ(actual, expected) << "Failed for input: " << input;
```

Shared test doubles live in `tests/cpp/TestDoubles/` and helpers in
`tests/cpp/tests_utils/` — check both before writing a new fake.

## Python Tests (pytest)

- `tests/python/` — unit tests for the launcher (`unit_launcher`). Plain
  pytest, no custom markers.
- `tests/end_to_end/examples/` — example-driven integration tests
  (`example_test.py`), selected by marker.

The duration markers (`short_sequential`, `short_mpi`, `short_memory`,
`medium_*`, `long_*`, `*_benders_by_batch_mpi`) are declared in
`tests/end_to_end/examples/pytest.ini` and apply **only** to that directory —
they do not exist in `tests/python/`. Running them requires `--installDir`:

```bash
cd tests/end_to_end/examples
pytest -m short_sequential --installDir=<xpansion-install-dir> example_test.py
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
directory. This is also the `WORKING_DIRECTORY` the `BDD` ctest target uses:

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
  (0 = resident subproblems, 1 = reload-from-disk with basis caching, 2 =
  compact skeleton representation — the last requires the study's `sub/`
  layout to actually be in the compact CSV format, not per-subproblem MPS)

## CI Requirements

- All tests must pass before merging
- Code coverage must not decrease significantly
- Run full test suite locally before pushing
