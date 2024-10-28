// Determine the degree of detail of the output, from 1 to 3
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 0, asInt())

// Maximum number of iterations accepted
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1, asInt())

// Absolute required level of precision
BENDERS_OPTIONS_MACRO(ABSOLUTE_GAP, double, 1, asDouble())

// Absolute required level of precision
BENDERS_OPTIONS_MACRO(RELATIVE_GAP, double, 1e-6, asDouble())

// Relative required level of precision with master relaxation
BENDERS_OPTIONS_MACRO(RELAXED_GAP, double, 1e-5, asDouble())

// In-out separation parameter
BENDERS_OPTIONS_MACRO(SEPARATION_PARAM, double, 0.5, asDouble())

// Formulation of the master problem
BENDERS_OPTIONS_MACRO(MASTER_FORMULATION, std::string, "integer", asString())

// True if cuts need to be aggregated, false otherwise
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false, asBool())

// Path to the folder where output files should be printed
BENDERS_OPTIONS_MACRO(OUTPUTROOT, std::string, ".", asString())

// True if a trace should be built, false otherwise
BENDERS_OPTIONS_MACRO(TRACE, bool, true, asBool())

// UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking
// each slave to its weight
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "CONSTANT", asString())

// If SLAVE_WEIGHT is CONSTANT, set here the divisor required
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT_VALUE, double, 1, asDouble())

// Name of the master problem file, if different from 'master'
BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master", asString())

// Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(STRUCTURE_FILE, std::string, "structure.txt", asString())

// Path to the folder where input files are stored
BENDERS_OPTIONS_MACRO(INPUTROOT, std::string, ".", asString())

// Name of the csv output file
BENDERS_OPTIONS_MACRO(CSV_NAME, std::string, "benders_output_trace", asString())

// True if alpha needs to be bounded by best upper bound, false otherwise
BENDERS_OPTIONS_MACRO(BOUND_ALPHA, bool, true, asBool())

// Name of solver to use
BENDERS_OPTIONS_MACRO(SOLVER_NAME, std::string, "COIN", asString())

// json file in output/expansion/
BENDERS_OPTIONS_MACRO(JSON_FILE, std::string, ".", asString())

// last iteration json file in output/expansion/
BENDERS_OPTIONS_MACRO(LAST_ITERATION_JSON_FILE, std::string, ".", asString())
// TIME_LIMIT
BENDERS_OPTIONS_MACRO(TIME_LIMIT, double, 1e12, asDouble())
// LAST_MASTER_MPS
BENDERS_OPTIONS_MACRO(LAST_MASTER_MPS, std::string, "master_last_iteration",
                      asString())
// Resume last benders
BENDERS_OPTIONS_MACRO(RESUME, bool, false, asBool())

// Name of the last master basis file
BENDERS_OPTIONS_MACRO(LAST_MASTER_BASIS, std::string, "master_last_basis",
                      asString())

// BATCH SIZE (Benders by batch)
BENDERS_OPTIONS_MACRO(BATCH_SIZE, size_t, 0, asUInt())

// is this an outer Loop
BENDERS_OPTIONS_MACRO(DO_OUTER_LOOP, bool, false, asBool())

// Outer Loop Options file
BENDERS_OPTIONS_MACRO(OUTER_LOOP_OPTION_FILE, std::string,
                      "adequacy_criterion.yml", asString())
// area file
BENDERS_OPTIONS_MACRO(AREA_FILE, std::string, "area.txt", asString())
