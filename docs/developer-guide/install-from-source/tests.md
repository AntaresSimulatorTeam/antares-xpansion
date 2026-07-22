# Tests

## CTests

Tests compilation  can be enabled at configure time using the option `-DBUILD_TESTING=ON` (`OFF` by default). After build, tests can be run with `ctest`:

```
cd _build
ctest -C Release --output-on-failure
```

All tests are associated to a label and multiple labels can be defined. You can choose which tests to execute when launching `ctest`. The list of available labels is the following:

| Name                                   | Label                                                 | Description                                                                                                 |
|:---------------------------------------|-------------------------------------------------------|-------------------------------------------------------------------------------------------------------------|
| `unit_logger`                          | `unit`                                                | Unit test for logger use.                                                                                   |
| `unit_launcher`                        | `unit`                                                | Unit test Antares Xpansion python launcher.                                                                 |
| `unit_solver`                          | `unit`                                                | Unit test of multisolver interface(COIN only).                                                              |
| `unit_lpnamer`                         | `unit`                                                | Unit test of lpnamer.                                                                                       |
| `unit_sensitivity`                     | `unit`                                                | Unit test for sensitivity analysis.                                                                         |
| `output_writer`                        | `unit`                                                | Unit test for the output writer.                                                                            |
| `helpers_test`                         | `unit`                                                | Unit test for helpers (json reader, AntaresVersionProvider).                                                |
| `lpnamer_end_to_end`                   | `lpnamer` `end_to_end`                                | End-to-end tests for lpnamer.                                                                               |
| `examples_short_sequential`            | `short` `short_sequential` `end_to_end`               | Non-parallel end-to-end tests using benders algorithm on antares studies (short duration).                  |
| `examples_short_memory`                | `short` `short_memory` `end_to_end`                   | Non-parallel end-to-end tests using benders algorithm on antares studies (short duration), use antares lib. |
| `examples_short_mpi`                   | `short` `short_mpi` `end_to_end`                      | Parallel end-to-end tests using benders algorithm on antares studies (short duration).                      |
| `examples_short_benders_by_batch_mpi`  | `short` `short_mpi` `end_to_end` `benders_by_batch`   | Parallel end-to-end tests using benders by batch algorithm on antares studies (short duration).             |
| `examples_medium_sequential`           | `medium` `medium_sequential` `end_to_end`             | Non-parallel end-to-end tests using benders algorithm on antares studies (medium duration).                 |
| `examples_medium_mpi`                  | `medium` `medium_mpi` `end_to_end`                    | Parallel end-to-end tests using benders algorithm on antares studies (medium duration).                     |
| `examples_medium_benders_by_batch_mpi` | `medium` `medium_mpi` `end_to_end` `benders_by_batch` | Parallel end-to-end tests using benders by batch algorithm on antares studies (medium duration).            |
| `examples_long_sequential`             | `long` `long_sequential` `end_to_end`                 | Non-parallel end-to-end tests using benders algorithm on antares studies (long duration).                   |
| `examples_long_mpi`                    | `long` `long_mpi` `end_to_end`                        | Parallel end-to-end tests using benders algorithm on antares studies (long duration).                       |
| `mpibenders`                           | `benders`, `benders-mpi` `end_to_end`                 | End-to-end tests for benders mpi optimization.                                                              |
| `sequential`                           | `benders` `benders-sequential` `end_to_end`           | End-to-end tests for benders sequential optimization.                                                       |
| `merge_mps`                            | `benders` `merge-mps` `end_to_end`                    | End-to-end tests for merge mps optimization.                                                                |

!!! Note
    Use `ctest -N` to see all available tests.

Here is an example for running only `examples_medium` tests (use of `Name` with `-R` option):

```
ctest -C Release --output-on-failure -R examples_medium
```

Here is an example for running only unit tests (use of `Label` with `-L` option):

```
ctest -C Release --output-on-failure -L unit
```

To run all test, don't indicate any label or name:

```
ctest -C Release --output-on-failure
```

## Cucumber Tests (BDD)

### Overview

Cucumber tests use the [behave](https://behave.readthedocs.io/) framework (Python BDD testing) to run end-to-end feature tests. These tests validate high-level functionality through human-readable scenarios.

### Location

```
tests/end_to_end/cucumber/
├── features/
│   ├── *.feature          # Feature files with Gherkin syntax
│   ├── environment.py     # Setup/teardown hooks
│   └── steps/             # Step implementations
│       ├── given.py
│       ├── when.py
│       ├── then.py
│       └── steps.py
```

### Running Tests

```bash
# Install dependencies
pip install -r requirements-tests.txt

# Run all cucumber tests
cd tests/end_to_end
behave cucumber/features/ --no-capture --no-capture-stderr

# Run specific feature file
behave cucumber/features/Benders.feature --no-capture

# Run with tags (skip flaky or CI-excluded tests)
behave cucumber/features/ --tags "not @flaky and not @noci" --no-capture

# Run specific scenario by name
behave cucumber/features/ -n "Benders_handle_mixed_order_of_var"
```

### Writing Tests

**Feature file** (`*.feature`):

```gherkin
Feature: Benders tests

  @fast @short @Benders
  Scenario: Test benders with specific study
    Given the study path is "data_test/Benders_handle_mixed_order_of_var"
    When I run benders with 1 proc(s)
    Then the simulation succeeds
    And the simulation takes less than 5 seconds
```

**Step implementations** are in the `steps/` directory. Each step definition maps Gherkin steps to Python code.

### Common Tags

| Tag | Description |
|-----|-------------|
| `@fast` | Quick tests |
| `@short` | Short duration |
| `@Benders` | Benders-related tests |
| `@flaky` | Unstable tests (skipped by default in CI) |
| `@noci` | Tests not run in CI |
| `@xpress` | Xpress solver-specific tests |

### GitHub Action

The workflow uses the action at `.github/workflows/cucumber-tests/action.yml` which:
- Accepts `feature` (specific feature file/folder, default: `features`)
- Accepts `tags` (tag expression, default: `"not @flaky and not @noci"`)
- Requires `mpi_path` for MPI support
- Runs `behave` with pretty formatting and no output capture