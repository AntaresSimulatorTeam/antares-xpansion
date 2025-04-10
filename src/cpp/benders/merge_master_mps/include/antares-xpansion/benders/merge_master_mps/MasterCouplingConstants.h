#pragma once


namespace master_structure 
{

// General data for the tree
constexpr char KEY_DATA[] = "data";
    constexpr char KEY_INITIAL_CAPACITIES[] = "initial_capacities";
        constexpr char KEY_DEFAULT[] = "default";

// Keys found inside each node's data :
    constexpr char KEY_INVESTMENT_DATE[] = "investment_date";
    constexpr char KEY_LP_FOLDER[] = "lp_folder";
    constexpr char KEY_MASTER_MPS_FILE[] = "master_mps_file";
    constexpr char KEY_STRUCTURE_FILE[] = "structure_file";
    constexpr char KEY_MASTER_NAME[] = "master_name";
    constexpr char KEY_PARENT[] = "parent";
    constexpr char KEY_WEIGHT_FACTOR[] = "weight_factor";
    constexpr char KEY_CONSTRAINTS[] = "constraints";
        constexpr char KEY_MAX_INVESTMENT[] = "max_investment";
        constexpr char KEY_MIN_INVESTMENT[] = "max_decommissioning";


// Forbidden name for a node :
constexpr char ROOT_NAME[] = "root";
// Default name of the master problem for each node
// Used when accessing the structure file (CouplingMap)
constexpr char DEFAULT_MASTER_NAME[] = "master";
}