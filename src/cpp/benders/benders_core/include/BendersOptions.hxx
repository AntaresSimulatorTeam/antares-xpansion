#include <string>
struct benders_options
{
    int
        // Determine the degree of detail of the output, from 1 to 3
        LOG_LEVEL = 1,
        // Maximum number of iterations accepted
        MAX_ITERATIONS = -1,
        // Number of slaves to use to solve the problem
        SLAVE_NUMBER = -1;

    double
        // Absolute required level of precision
        ABSOLUTE_GAP = 1,
        // Absolute required level of precision
        RELATIVE_GAP = 1e-12,
        // If SLAVE_WEIGHT is CONSTANT, set here the divisor required
        SLAVE_WEIGHT_VALUE = 1,
        // TIME_LIMIT
        TIME_LIMIT = 1e12;

    bool
        // True if cuts need to be aggregated, false otherwise
        AGGREGATION = false,
        // True if a trace should be built, false otherwise
        TRACE = true,
        // True if alpha needs to be bounded by best upper bound, false otherwise
        BOUND_ALPHA = true;

    std::string
        // Path to the folder where output files should be printed
        OUTPUTROOT = ".",
        // UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight
        SLAVE_WEIGHT = "CONSTANT",
        // Name of the master problem file, if different from 'master'
        MASTER_NAME = "master",
        // Number of slaves to use to solve the problem
        STRUCTURE_FILE = "structure.txt",
        // Path to the folder where input files are stored
        INPUTROOT = ".",
        // Name of the csv output file
        CSV_NAME = "benders_output_trace",
        // Name of solver to use
        SOLVER_NAME = "COIN",
        // json file in output/expansion/
        JSON_FILE = ".";
};
// Determine the degree of detail of the output, from 1 to 3
BENDERS_OPTIONS_MACRO(LOG_LEVEL, int, 0)

// Maximum number of iterations accepted
BENDERS_OPTIONS_MACRO(MAX_ITERATIONS, int, -1)

// Absolute required level of precision
BENDERS_OPTIONS_MACRO(ABSOLUTE_GAP, double, 1)

// Absolute required level of precision
BENDERS_OPTIONS_MACRO(RELATIVE_GAP, double, 1e-12)

// True if cuts need to be aggregated, false otherwise
BENDERS_OPTIONS_MACRO(AGGREGATION, bool, false)

// Path to the folder where output files should be printed
BENDERS_OPTIONS_MACRO(OUTPUTROOT, std::string, ".")

// True if a trace should be built, false otherwise
BENDERS_OPTIONS_MACRO(TRACE, bool, true)

// UNIFORM (1/n), CONSTANT (to set in SLAVE_WEIGHT_VALUE), or a txt file linking each slave to its weight
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT, std::string, "CONSTANT")

// If SLAVE_WEIGHT is CONSTANT, set here the divisor required
BENDERS_OPTIONS_MACRO(SLAVE_WEIGHT_VALUE, double, 1)

// Name of the master problem file, if different from 'master'
BENDERS_OPTIONS_MACRO(MASTER_NAME, std::string, "master")

// Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(SLAVE_NUMBER, int, -1)

// Number of slaves to use to solve the problem
BENDERS_OPTIONS_MACRO(STRUCTURE_FILE, std::string, "structure.txt")

// Path to the folder where input files are stored
BENDERS_OPTIONS_MACRO(INPUTROOT, std::string, ".")

// Name of the csv output file
BENDERS_OPTIONS_MACRO(CSV_NAME, std::string, "benders_output_trace")

// True if alpha needs to be bounded by best upper bound, false otherwise
BENDERS_OPTIONS_MACRO(BOUND_ALPHA, bool, true)

// Name of solver to use
BENDERS_OPTIONS_MACRO(SOLVER_NAME, std::string, "COIN")

// json file in output/expansion/
BENDERS_OPTIONS_MACRO(JSON_FILE, std::string, ".")
// TIME_LIMIT
BENDERS_OPTIONS_MACRO(TIME_LIMIT, double, c)
