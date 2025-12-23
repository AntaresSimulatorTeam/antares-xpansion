# Setting of launching benders exec 


It is possible to launch the Benders solver directly as a standalone application. The solver is provided as an executable named benders, which can be invoked from the command line without requiring any additional wrapper or interface.

The execution of the solver is configured through an input file named options.json, which must be supplied at runtime. This JSON configuration file defines all the parameters required to control the behavior of the solver. It typically includes the solver’s operational settings, algorithmic options, and problem-specific configurations. By adjusting the attributes in this file, users can fine-tune aspects such as decomposition settings, convergence criteria, logging behavior, performance-related options, and input/output paths.

This design allows for a clear separation between the solver logic and its configuration, making the execution flexible, reproducible, and easy to automate within scripts or larger workflows. The following attributes are defined in the options.json file:

| Name | Default value | Description |
| -----| -------------| -------------|
|MAX_ITERATIONS | `-1` | The maximum number of benders iteration |
|RELATIVE_GAP | `1e-6` | Tolerance on relative gap  |
|ABSOLUTE_GAP | `1` | Tolerance on absolute gap  |
|RELAXED_GAP | `1e-5` | Level of precision with master relaxation   |
|AGGREGATION | `1` | Number of added cuts at each master iteration   |
|MICRO_ITERATION | `false` | Level of precision with master relaxation   |
|OUTPUT | `.` | Folder where output files should be printed   |
|TRACE | `true` | Checking if trace should be build or not  |
|SLAVE_WEIGHT | `CONSTANT` | Weights for subproblem   |
|SLAVE_WEIGHT_VALUE | `1` | If SLAVE_WEIGHT is CONSTANT, set here the divisor required  |
|MASTER | `master` | Name of the master problem file, if different from 'master'  |
|STRUCTURE_FILE | `structure.txt` | Number of slaves to use to solve the problem   |
|INPUTROOT | `.` | Path to the folder where input files are stored  |
|CSV_NAME | `benders_output_trace` | Name of the csv output file |
|BOUND_ALPHA | `true` | True if alpha needs to be bounded by best upper bound, false otherwise |
|SEPARATION_PARAM | `0.5` | In-out separation parameter |
|BATCH_SIZE | `0` | Size of batch in benders by batch algorithm|
|JSON_FILE | `.` | Json file in output/expansion/ |
|LAST_ITERATION_JSON_FILE | `.` | Last iteration json file in output/expansion/|
|MASTER_FORMULATION | `integer` | Formulation of the master problem |
|SOLVER_NAME | `COIN` | Name of solver to use |
|TIME_LIMIT | `1e12` | Simulation time limit |
|LOG_LEVEL | `0` | The degree of detail of the output, from 1 to 3  |
|LAST_MASTER_MPS | `master_last_iteration` | Results of the last iteration |
|LAST_MASTER_BASIS | `master_last_basis` |Results of the last iteration|
|DO_OUTER_LOOP | `0` | Outer loop  |

