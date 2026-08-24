# Settings for launching Benders exec 


It is possible to launch the Benders solver directly as a standalone application. The solver is provided as an executable named Benders, which can be invoked from the command line without requiring any additional wrapper or interface.

The execution of the solver is configured through an input file named `options.json`, which must be supplied at runtime. This JSON configuration file defines all the parameters required to control the behavior of the solver. It typically includes the solver’s operational settings, algorithmic options, and problem-specific configurations. By adjusting the attributes in this file, users can fine-tune aspects such as decomposition settings, convergence criteria, logging behavior, performance-related options, and input/output paths.

This design allows for a clear separation between the solver logic and its configuration, making the execution flexible, reproducible, and easy to automate within scripts or larger workflows. The following attributes are defined in the options.json file:

| Name | Default value | Description | Possible values |
| -----| -------------| -------------|-------------|
|MAX_ITERATIONS | `-1` | The maximum number of Benders iteration |`integer` >= -1|
|RELATIVE_GAP | `1e-6` | Tolerance on relative gap  | `double` > 0|
|ABSOLUTE_GAP | `1` | Tolerance on absolute gap  |`double` > 0 |
|RELAXED_GAP | `1e-5` | Level of precision with master relaxation   |`double` > 0|
|NB_CUTS_PER_ITER | `1` | Number of added cuts at each master iteration   | `unsigned integer` |
|OUTPUT | `.` | Folder where output files should be printed   | `string` | 
|TRACE | `true` | Checking if trace should be build or not  |`boolean` |
|SLAVE_WEIGHT | `CONSTANT` | Weights for subproblem   |`UNIFORM` , `CONSTANT` or  txt file linking|
|SLAVE_WEIGHT_VALUE | `1` | If SLAVE_WEIGHT is CONSTANT, set here the divisor required  | `double` |
|MASTER_NAME | `master` | Name of the master problem file, if different from 'master'  | `string` |
|STRUCTURE_FILE | `structure.txt` | Number of slaves to use to solve the problem    | `string` |
|INPUTROOT | `.` | Path to the folder where input files are stored  |`string` |
|CSV_NAME | `benders_output_trace` | Name of the csv output file | `string`|
|BOUND_ALPHA | `true` | True if alpha needs to be bounded by best upper bound, false otherwise | `boolean` |
|SEPARATION_PARAM | `0.5` | In-out separation parameter |`double` between `0` and `1`  |
|BATCH_SIZE | `0` | Size of batch in Benders by batch algorithm| `unsigned integer` |
|JSON_FILE | `.` | Path of the JSON output file (absolute or relative to `INPUTROOT`) | `string` |
|LAST_ITERATION_JSON_FILE | `.` | Path of the last iteration JSON file (absolute or relative to `INPUTROOT`)| `string` |
|MASTER_FORMULATION | `integer` | Formulation of the master problem | `integer` or `relaxed`|
|SOLVER_NAME | `COIN` | Name of solver to use | `XPRESS`, `COIN`|
|TIME_LIMIT | `1e12` | Simulation time limit | `double` > 0|
|LOG_LEVEL | `0` | The degree of detail of the output |from `0`, `1`, `2` |
|LAST_MASTER_MPS | `master_last_iteration` | Name of the master MPS file of the last iteration | `string`|
|LAST_MASTER_BASIS | `master_last_basis` | Name of the file to write the basis of the master at the last iteration| `string`|
|DO_OUTER_LOOP | `false` |  Whether to perform outer loop  |`boolean`|
|OUTPUTROOT | `.` |  Path to the folder where output files should be printed  | `boolean` |
|OUTER_LOOP_OPTION_FILE | `adequacy_criterion.yml` |  Outer Loop Options file  |`string`|
|AREA_FILE | `area.txt` |  Area file used to get areas on which external criteria (LOLD, UnsuppliedEnergy) are computed |`string` |
|CACHE_PROBLEMS | `0` |  Subproblem caching mode: `0` = all problems loaded in memory (classic mode), `1` = subproblems stored in disk cache rather than loaded in memory (reduces RAM usage at the expense of a slight CPU time performance reduction), `2` = memory-optimized mode using a shared skeleton problem with per-subproblem CSV data  | `0`, `1` or `2` |
|PROBLEMS_FORMAT | `MPS_FILE` |  Format of the problems |`MPS_FILE` or `OPTIMIZED` |
|MASTER_SOLUTION_TOLERANCE | `1e-4` |  Tolerance for rounding the solution variables of the master problem (to avoid subproblems infeasibilities) | `double` > 0|
|CUT_COEFFICIENT_TOLERANCE | `5e-3` |  Cofficient under which cuts coefficients and right-hand sides are considered to be zero | `double` > 0|
|KEEP_FULL | `false` |  Flag to store full problems after presolve | `boolean`|
|FULL_DIR | `full` |  Full problems directory name|`string` |
|RESUME | `false` |  Resume last Benders | `boolean`|
|MICRO_ITERATIONS | `false` |  Whether to enable the micro-iterations mode, which applies constraint generation at the subproblem level to reduce problem size and speed up resolution | `boolean`|