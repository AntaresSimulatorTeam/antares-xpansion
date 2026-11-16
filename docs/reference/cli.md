# Antares Xpansion launcher

## Quick start

1. Open a command prompt in your Antares Xpansion install directory (by default it is named
   `antaresXpansion-x.y.z-win64`
   where `x.y.z` is the version number).

2. Run `antares-xpansion-launcher.exe` and choose the path to the
   Antares study with the `-i` parameter:
   ```
   antares-xpansion-launcher.exe -i examples\SmallTestFiveCandidates
   ```

> The `-i` parameter can also be replaced by `--dataDir`.

## Command line parameters

### `-h, --help`

Show a help message and exit.

### `--step {full, antares, problem_generation, benders, gems, study_update, sensitivity, presolve, resume}`

Default value: `full`.

The execution of Antares Xpansion consists of several steps that can be run separately. The
`--step` parameter allows to select the steps to execute:

| Step                 | Description                                                                                                                                         |
|:---------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------|
| `antares`            | Launch Antares-Simulator once to get the Antares problem.                                                                                           |
| `problem_generation` | Generate the full Antares Xpansion problem using the user input and the output of the Antares-Simulator run.                                        |
| `benders`            | Solve the investment optimization problem of Antares Xpansion, using the benders decomposition.                                                     |
| `gems`               | Combined step: runs `antares-problem-generator` followed by `benders`. antares-problem-generator creates problems files using GEMS optim-config     |
| `study_update`       | Update the Antares study with the solution returned by the benders decomposition algorithm.                                                         |
| `full`               | Launch all steps in order: `antares` > `problem_generation` > `benders` > `study_update`                                                            |
| `sensitivity`        | Launch sensitivity analysis.                                                                                                                        |
| `presolve`           | Launch a presolver on the decomposed problem (only available with Xpress solver).                                                                   |
| `resume`             | resume benders step of a study in accordance with `--simulationName`, by default `last` study is resumed.                                           |

### `-i, --dataDir`

Specify the Antares study folder path. Use quotes `“antares study path”` in case of a space in the path.

### `--simulationName {last, your-antares-output-directory}`

Default value: `last`.

Define the name of the Antares-Simulator output archive that Antares Xpansion uses to generate the expansion problem.
If the value is `last`, the most recent run will be used. This option only has an effect when `--step` is
among `{problem_generation, benders, study_update, sensitivity}`.
In a step by step workflow keep both _.zip_ file and _-Xpansion_ corresponding folder.

### `-m, --method {benders, mergeMPS, adequacy_criterion}`

Default value: `benders`.

Set the optimization method used by Antares Xpansion.

| Option               | Description                                                                                                                                            |
|----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| `benders`            | Launch the classical Benders decomposition or the Benders by batch algorithm depending on `batch_size`.
| `mergeMPS`           | Launch a frontal resolution of the investment problem (i.e. without decomposition). This is much more time-consuming than using Benders decomposition. |
| `adequacy_criterion` | Launch Antares Xpansion with reliability constraints, see adequacy criterion.                                                 |

### `--problem-format {mps, OPTIMIZED}`

Default value: `OPTIMIZED`.

Select the storage format of the generated mathematical problems (master + subproblems):

- `OPTIMIZED` (default) : use underlying solver to write problems in an optimized format to reduce disk space usage and
  I/O time.
  The underlying format depends on the solver used.
    - _XPRESS_ : `svf` format: compressed binary format.
    - _COIN_ : unsupported. Falls back to `mps`.
- `mps` : write the problems in MPS format, which is a standard format for mathematical programming problems.

#### `-n, --np`

Default value: 2.

Sets the number of MPI processes to use for the Benders decomposition. This option only has an effect when `-m` is set
to `benders`.

### `--antares-n-cpu`

Default value: 1.

Sets the number of threads to use for Antares-Simulator in the initial `antares` step.

### `--keepMps`

Default value: `False`.

If set, keeps `mps` files that encodes the problems solved by the optimizer. This option must be set
if the user intends to launch the optimization several times on the same study (`--step benders`) without doing the
other steps of Antares Xpansion.

### `--presolve`

Default value: `False`.

If set, runs the `presolve` step before calling the `benders` step

### `-v, --version`

Show the Antares Xpansion version.

### `--antares-version`

Show the Antares-Simulator version (used in the `antares` step).

### `--cache_problems {NO_CACHE, PER_SUB, COMPACT}`

Default value: `NO_CACHE`.

Select how the Benders subproblems are held during the `benders` step. This is useful when hitting memory or disk
limitations, at the cost of some processing time:

- `NO_CACHE`: all subproblems are loaded in memory (classic mode).
- `PER_SUB`: each subproblem is loaded from disk just before being solved, which reduces RAM usage.
- `COMPACT`: a single shared skeleton problem plus lightweight per-subproblem CSV files are used, which also reduces
  the disk footprint. See [in-memory compact subproblems](benders/in-memory-compact-subproblems.md).

### `--memory`

Default value: `False`.

Work in memory and avoid writing files when possible. Mainly useful for the `problem_generation` step, and required
when generating problems for [trajectory investment](trajectory-investment/orchestration.md).

### `--trajectory`

Default value: `False`.

Switch to trajectory mode. All the remaining arguments are then parsed by the trajectory argument parser; use
`--trajectory --help` to list the trajectory-specific options. See
[trajectory investment orchestration](trajectory-investment/orchestration.md).

### `--oversubscribe`

Default value: `False`.

Enable the MPI `--oversubscribe` option, which allows launching more MPI processes than available cores (Linux only).

### `--allow-run-as-root`

Default value: `False`.

Enable the MPI `--allow-run-as-root` option, needed to run as the root user, e.g. in a container (Linux only).

### `--revision`

Show the abbreviated commit hash the launcher was built from, and exit.
