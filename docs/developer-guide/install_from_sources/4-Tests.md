# Tests

Tests compilation  can be enabled at configure time using the option `-DBUILD_TESTING=ON` (`OFF` by default). After build, tests can be run with `ctest`:

!!! Note
    You need mpirun in your path for several tests.
    You can use the one build by vcpkg in packages/openmpi_x64-linux-release/tools/openmpi/bin/
    Exemple:
    ```
    export PATH=$PATH:/path/to/vcpkg/packages/openmpi_x64-linux-release/tools/openmpi/bin/
    ```

```
cd _build
ctest -C Release --output-on-failure
```

All tests are associated to a label and multiple labels can be defined. You can choose which tests to execute when launching `ctest`. The list of available labels is the following:

| Name                                   | Label                                                 | Description                                                                                                 |
|:---------------------------------------|-------------------------------------------------------|-------------------------------------------------------------------------------------------------------------|
| `unit_logger`                          | `unit`                                                | Unit test for logger use.                                                                                   |
| `unit_launcher`                        | `unit`                                                | Unit test Antares-Xpansion python launcher.                                                                 |
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
