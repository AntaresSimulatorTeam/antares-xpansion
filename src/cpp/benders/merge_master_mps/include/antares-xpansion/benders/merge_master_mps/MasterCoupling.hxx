#pragma once

#include <string>

namespace master_structure 
{
// General data for the tree
inline constexpr const char* KEY_DATA = "data";
    inline constexpr const char* KEY_INITIAL_CAPACITIES = "initial_capacities";
        inline constexpr const char* KEY_DEFAULT = "default";

// Keys found inside the node data :
    inline constexpr const char* KEY_INVESTMENT_DATE = "investment_date";
    inline constexpr const char* KEY_LP_FOLDER = "lp_folder";
    inline constexpr const char* KEY_MASTER_MPS_FILE = "master_mps_file";
    inline constexpr const char* KEY_STRUCTURE_FILE = "structure_file";
    inline constexpr const char* KEY_MASTER_NAME = "master_name";
    inline constexpr const char* KEY_PARENT = "parent";
    inline constexpr const char* KEY_WEIGHT_FACTOR = "weight_factor";
    inline constexpr const char* KEY_CONSTRAINTS = "constraints";
        inline constexpr const char* KEY_MAX_INVESTMENT = "max_investment";
        inline constexpr const char* KEY_MIN_INVESTMENT = "max_decommissioning";


// Default values for some fields
// Forbidden name for a node :
inline constexpr const char* ROOT_NAME = "root";
inline constexpr const char* DEFAULT_MASTER_NAME = "master";
}