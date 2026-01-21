# Water values computation

The inputs for the computation of the water values are:

- a study
- a grid defined by a grid.csv file at the root of the study

The outputs are the water values for all the weeks discretized over 101 level of stock for all the grid IDs. The output is a json file named `[grid_ID]_water_values.json` located in the output directory.

## Input file grid.csv

Here is an example of a grid.csv file (must be comma separated, shown here as a table for readability):

|grid_id|problem_name|type|name|area|min|max|step|
|-|-|-|-|-|-|-|-|
|0|all|constraint|HydroPower|area|0|1|0.1|

with the following columns:

- `grid_id`: the ID of the grid
- `problem_name`: the name of the problem (`all` if all the problems are considered)
- `type`: the type of the constraint (`constraint` or `variable`)
- `name`: the name of the constraint or variable (`HydroPower` for Bellman values computation)
- `area`: the area to take into account
- `min`: the minimum value of the constraint or variable (from 0 to 1)
- `max`: the maximum value of the constraint or variable (from 0 to 1)
- `step`: the step of the discretization of the constraint or variable (from 0 to 1)

### Secondary input file penalties.yaml

Here is an example of a penalties.yaml file, that defines parameters related to the penalties when computing water values:

```yaml
# All parameters related to penalties when computing water values.
# Use ~ to fall back on default values (as implemented in C++ code)
# default values are subject to change

penalty_bottom_rule_curve : 0
# default: 0

penalty_upper_rule_curve : 0
# default: 0

penalty_final_level : 2000
# default: 0

force_final_level : true
# default: false

final_level : ~
# default: initial level

cvar : 0.8
# default: 1.0 (all scenarios taken into account for Bellman values)
# will be restricted to [0.0 ; 1.0]
```

This file is expected to be located at the root of the study. It is optional, however default values are hard-coded in the program.

## Command line usage

### Quick start

1. Open a command prompt in your Antares-Xpansion install directory (by default it is named `antaresXpansion-x.y.z-win64` where `x.y.z` is the version number).
    You can launch a command line prompt by typing `cmd` in the path.

2. Run `water-values.exe` and choose the path to the Antares study with the `--study` parameter :

    ```cmd
    water_values_exe --study data_test/one_node_base
    ```

### Command line parameters

#### `-h, --help`

Show a help message and exit.

#### `--study <path>`

Path to the Antares study.

#### `--solver {xpress, coin}`

Default value: `xpress`.

#### `--threads <number>`

Default value: `1`.

Number of threads that will be used to solve the problems from the grid.

#### `--start-week <number>`

Default value: `1`.

Starting week of the Bellman values computation.

#### `--end-week <number>`

Default value: `52`.

Ending week of the Bellman values computation.

#### `--nb-levels <number>`

Default value: `10`.

Number of levels of the stock.

#### `--antares-format <bool>`

Default value: `false`.

If true, the output will be in the Antares format (values will be interpolated to get 101 levels of stock).

#### `--keepMps <bool>`

Default value: `false`.

If true, a file (.mps or .svf, see below) for each solved problem will be written to disk (location is `<study>/output/<run>/mps_n`).

#### `--problem-format {mps, optimized}`

Default value: `optimized`.

Selects the storage format of the generated mathematical problems (master + subproblems):

- OPTIMIZED (default) : use underlying solver to write problems in an optimized format to reduce disk space usage and I/O time. The underlying format depends on the solver used.
  - XPRESS : svf format: compressed binary format.
  - COIN : unsupported. Falls back to MPS.
- MPS : write the problems in MPS format, which is a standard format for mathematical programming problems.

## Workflow

![Water values computation workflow](../../assets/media/water-values-workflow.png)
