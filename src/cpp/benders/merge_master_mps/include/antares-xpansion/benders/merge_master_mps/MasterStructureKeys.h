#pragma once

namespace MasterStructureKeys
{

// Types of candidates variables
constexpr char VARIABLE_X[] = "x";
constexpr char VARIABLE_DXPLUS[] = "dx_plus";
constexpr char VARIABLE_DXMINUS[] = "dx_minus";

// Types of constraints
constexpr char CONSTRAINT_EQUALS[] = "=";
constexpr char CONSTRAINT_LEQ[] = "<";
constexpr char CONSTRAINT_GEQ[] = ">";

// General data for the tree
constexpr char KEY_INITIAL_CAPACITIES[] = "initial_capacities";
constexpr char KEY_DEFAULT[] = "default";

// Candidates costs
constexpr char KEY_CANDIDATES_TYPES[] = "candidates_types";
constexpr char KEY_OPERATION_COST[] = "operation_maintenance";
constexpr char KEY_INVESTMENT_COST[] = "investment";
constexpr char KEY_RETIREMENT_COST[] = "retirement";

// Constraints
constexpr char KEY_CONSTRAINTS[] = "constraints";
constexpr char KEY_COEFFICIENTS[] = "coeffs";
constexpr char KEY_RHS[] = "rhs";
constexpr char KEY_OPERATOR[] = "operator";

constexpr char KEY_TREE[] = "tree";
// Keys found inside each node's data :
constexpr char KEY_INVESTMENT_DATE[] = "investment_date";
constexpr char KEY_LP_FOLDER[] = "lp_folder";
constexpr char KEY_MASTER_MPS_FILE[] = "master_mps_file";
constexpr char KEY_STRUCTURE_FILE[] = "structure_file";
constexpr char KEY_MASTER_NAME[] = "master_name";
constexpr char KEY_PARENT[] = "parent";
constexpr char KEY_WEIGHT_FACTOR[] = "weight_factor";
constexpr char KEY_CANDIDATES[] = "candidates_costs";

// Forbidden name for a node :
constexpr char ROOT_NAME[] = "root";

// Default name of the master problem for each node
// Used when accessing the structure file (CouplingMap)
constexpr char DEFAULT_MASTER_NAME[] = "master";
constexpr char DEFAULT_STRUCTURE_FILE[] = "structure.txt";
} // namespace MasterStructureKeys
