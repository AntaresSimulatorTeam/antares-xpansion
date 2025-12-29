# Settings for launching Benders exec 


It is possible to launch the Benders solver directly as a standalone application. The solver is provided as an executable named Benders, which can be invoked from the command line without requiring any additional wrapper or interface.

The execution of the solver is configured through an input file named `options.json`, which must be supplied at runtime. This JSON configuration file defines all the parameters required to control the behavior of the solver. It typically includes the solver’s operational settings, algorithmic options, and problem-specific configurations. By adjusting the attributes in this file, users can fine-tune aspects such as decomposition settings, convergence criteria, logging behavior, performance-related options, and input/output paths.

This design allows for a clear separation between the solver logic and its configuration, making the execution flexible, reproducible, and easy to automate within scripts or larger workflows. The following attributes are defined in the options.json file:

| Name | Default value | Description |
| -----| -------------| -------------|
|MAX_ITERATIONS | `-1` | The maximum number of Benders iteration |
|RELATIVE_GAP | `1e-6` | Tolerance on relative gap  |
|ABSOLUTE_GAP | `1` | Tolerance on absolute gap  |
|RELAXED_GAP | `1e-5` | Level of precision with master relaxation   |
|AGGREGATION | `1` | Number of added cuts at each master iteration   |
|OUTPUT | `.` | Folder where output files should be printed   |
|TRACE | `true` | Checking if trace should be build or not  |
|SLAVE_WEIGHT | `CONSTANT` | Weights for subproblem   |
|SLAVE_WEIGHT_VALUE | `1` | If SLAVE_WEIGHT is CONSTANT, set here the divisor required  |
|MASTER_NAME | `master` | Name of the master problem file, if different from 'master'  |
|STRUCTURE_FILE | `structure.txt` | Number of slaves to use to solve the problem    |
|INPUTROOT | `.` | Path to the folder where input files are stored  |
|CSV_NAME | `benders_output_trace` | Name of the csv output file |
|BOUND_ALPHA | `true` | True if alpha needs to be bounded by best upper bound, false otherwise |
|SEPARATION_PARAM | `0.5` | In-out separation parameter |
|BATCH_SIZE | `0` | Size of batch in Benders by batch algorithm|
|JSON_FILE | `.` | Path of the JSON output file (absolute or relative to `INPUTROOT`) |
|LAST_ITERATION_JSON_FILE | `.` | Path of the last iteration JSON file (absolute or relative to `INPUTROOT`)|
|MASTER_FORMULATION | `integer` | Formulation of the master problem |
|SOLVER_NAME | `COIN` | Name of solver to use |
|TIME_LIMIT | `1e12` | Simulation time limit |
|LOG_LEVEL | `0` | The degree of detail of the output, from 0 to 2 |
|LAST_MASTER_MPS | `master_last_iteration` | Name of the master MPS file of the last iteration |
|LAST_MASTER_BASIS | `master_last_basis` | Name of the file to write the basis of the master at the last iteration|
|DO_OUTER_LOOP | `false` |  Whether to perform outer loop  |
|OUTPUTROOT | `.` |  Path to the folder where output files should be printed  |
|OUTER_LOOP_OPTION_FILE | `adequacy_criterion.yml` |  Outer Loop Options file  |
|AREA_FILE | `area.txt` |  Area file used to get areas on which external criteria (LOLD, PositiveUnsuppliedEnergy) are computed |
|CACHE_PROBLEMS | `false` |  Whether to use subproblems in disk cache rather than loading all problems in memory (allows to reduce RAM usage at the expense of a slight CPU time performance reduction)  |
|PROBLEMS_FORMAT | `MPS_FILE` |  Format of the problems |
|MASTER_SOLUTION_TOLERANCE | `1e-4` |  Tolerance for rounding the solution variables of the master problem (to avoid subproblems infeasibilities) |
|CUT_COEFFICIENT_TOLERANCE | `5e-3` |  Cofficient under which cuts coefficients and right-hand sides are considered to be zero |
|KEEP_FULL | `false` |  Flag to store full problems after presolve |
|FULL_DIR | `full` |  Full problems directory name|
|RESUME | `false` |  Resume last Benders |


