//Determine the degree of detail of the output, from 1 to 3
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 3)

//Maximum number of iterations accepted
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1)

//Level of precision accepted
BENDERS_OPTIONS_MACRO(GAP, double, 1e-6)

//True if cuts need to be aggregated, false otherwise
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false)

//Path to the folder where output files should be printed
BENDERS_OPTIONS_MACRO(OUTPUTROOT, std::string, ".")

//True if a trace should be built, false otherwise
BENDERS_OPTIONS_MACRO(TRACE, bool, true)

//True if similar cuts should be deleted, false otherwise
BENDERS_OPTIONS_MACRO(DELETE_CUT, bool, false)

//UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "CONSTANT")

//If SLAVE_WEIGHT is CONSTANT, set here the divisor required
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT_VALUE, double, 1)

//Name of the master problem file, if different from 'master'
BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master")

//Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(SLAVE_NUMBER, int, -1)

//Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(STRUCTURE_FILE, std::string, "structure.txt")

//Path to the folder where input files are stored
BENDERS_OPTIONS_MACRO(INPUTROOT, std::string, ".")

//True if simplex bases need to be stored, false otherwise
BENDERS_OPTIONS_MACRO(BASIS, bool, true)

//True if a statement of active cuts need to be done, false otherwise
BENDERS_OPTIONS_MACRO(ACTIVECUTS, bool, false)

//Number of iterations before aggregation of all previous cuts, set to 0 if no aggregation needed
BENDERS_OPTIONS_MACRO(THRESHOLD_AGGREGATION, int, 0)

//Number of iterations before aggregation of all previous cuts by iteration, set to 0 if no aggregation needed
BENDERS_OPTIONS_MACRO(THRESHOLD_ITERATION, int, 0)

//Number of slaves to select for random aggregation, set to 0 if no random aggregation needed
BENDERS_OPTIONS_MACRO(RAND_AGGREGATION, int, 0)

//Name of the csv output file
BENDERS_OPTIONS_MACRO(CSV_NAME, std::string, "benders_output_trace")

//True if alpha needs to be bounded by best upper bound, false otherwise
BENDERS_OPTIONS_MACRO(BOUND_ALPHA, bool, true)
