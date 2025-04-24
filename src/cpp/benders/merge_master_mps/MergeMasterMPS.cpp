#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/merge_master_mps/MasterCouplingConstants.h"


#include <filesystem>
#include <utility>
#include <numeric>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

MergeMasterTrajectoryMPS::TrajectoryGlobalData::TrajectoryGlobalData(const Json::Value& data)
{
    using namespace MasterCouplingConstants;

    const auto& initial_capacities_data = data[KEY_INITIAL_CAPACITIES];
    // Set a default default value
    initial_capacities[KEY_DEFAULT] = 0;
    for (const auto& candidate_name : initial_capacities_data.getMemberNames())
    {
        initial_capacities[candidate_name] = initial_capacities_data[candidate_name].asDouble();
    }
}

MergeMasterTrajectoryMPS::TrajectoryNode::TrajectoryNode(const std::string& node, const Json::Value& data) :
    name{node}
{
    using namespace MasterCouplingConstants;

    path = data[KEY_LP_FOLDER].asString();
    master_mps_file = data[KEY_MASTER_MPS_FILE].asString();
    structure_file = data[KEY_STRUCTURE_FILE].asString();
    if (data.isMember(KEY_PARENT))
    {
        parent = data[KEY_PARENT].asString();

        // Compatibility for root given as hardcoded name
        if (parent == ROOT_NAME)
        {
            parent = std::nullopt;
        }
    }
    weight = data[KEY_WEIGHT_FACTOR].asDouble();
    
    // If a MASTER_NAME is given, set it (used when accesing the structure file)
    if (data.isMember(KEY_MASTER_NAME))
    {
        master_name = data[KEY_MASTER_NAME].asString();
    }

    // Constraints TBA
}

void MergeMasterTrajectoryMPS::read_master_structure(const std::filesystem::path& path) 
{
    using namespace MasterCouplingConstants;
    
    const auto input = get_json_file_content(path);

    // Read the node by node data
    for (const auto& node_name : input.getMemberNames())
    {
        if (node_name == KEY_DATA)
            continue;

        const auto& node_data = input[node_name];
        tree_.emplace_back(TrajectoryNode(node_name, node_data));
    }

    logger_->display_message("Master coupling map generated successfully.");
    logger_->display_message("Number of nodes: " + std::to_string(tree_.size()));

    // Read the global trajectory data
    const auto& general_data = input[KEY_DATA];
    trajectory_data_ = TrajectoryGlobalData(general_data);
}


std::string MergeMasterTrajectoryMPS::make_prefix_from_node(const std::string& node_name) const
{
    return "node_" + node_name + "__";
}

double MergeMasterTrajectoryMPS::get_candidate_initial_value(const std::string& candidate) const
{
    using namespace MasterCouplingConstants;

    double initial_value = 0;
    auto it = trajectory_data_.initial_capacities.find(candidate);
    if (it != trajectory_data_.initial_capacities.end())
    {
        initial_value = it->second;
    }
    else
    {
        logger_->display_message("Did not find candidate " + candidate + " 's initial value, looking up key" + KEY_DEFAULT);
        initial_value = trajectory_data_.initial_capacities.at(KEY_DEFAULT);
    }

    return initial_value;
}

void MergeMasterTrajectoryMPS::build_problem()
{
    logger_->display_message("Inside MergeMasterTrajectoryMPS::build_problem()");
    logger_->display_message("Trying to parse structure file at " + std::string(tree_path_));

    read_master_structure(tree_path_);

    // Check that the problem format is compatible with the solver
    if(options_.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE
        && options_.SOLVER_NAME != "Xpress")
    {
        std::cerr << LOGLOCATION <<
            "Invalid solver used with the saved file format" << options_.SOLVER_NAME << "\n" <<
            "Can only use Xpress with this option" << std::endl;
    }

    logger_->display_message("Merging master problems...");

    for (const auto& node_data : tree_)
    {
        const std::string& node_name = node_data.name;

        SolverAbstract::Ptr solver_local = get_local_solver(options_.INPUTROOT / node_data.path, node_data.master_mps_file);
        solver_local->set_output_log_level(options_.LOG_LEVEL);

        // Read the problem
        logger_->display_message("Reading problem " + (options_.INPUTROOT / node_data.path / node_data.master_mps_file).string());

        // Multiply the objective function by the weight factor
        double weight_factor = node_data.weight;
        logger_->display_message("Weight factor for node " + node_name + " : " + std::to_string(weight_factor));
        AbstractMergeMPS::multiply_obj_by_weight_factor(*solver_local, weight_factor);

        StandardLp lpData(*solver_local);
        std::string varPrefix_local = make_prefix_from_node(node_name);

        // Perhaps we could think of a way to include / use AbstractMergeMPS::merge_local_solver
        // But it does not do exactly what we do here for now, particularily when building the new structure file
        // It returns a map : old_var_name -> new_position
        // Where as we are building : master -> prefixed_var_name -> new_position
        // And : subproblem -> prefixed_var_name -> unchanged_position
        lpData.append_in(*ptr_merged_solver_, varPrefix_local);

        // Load the coupling map (structure file) for this node
        // It will be used to iterate through the investment candidates in the node and get their position in the merged problem
        CouplingMap node_coupling_map = CouplingMapGenerator::BuildInput(
            std::filesystem::path(options_.INPUTROOT) / node_data.path / node_data.structure_file,
            logger_.get()
        );

        // Second step : get the candidate's position in the merged problem
        for (const auto& [candidate_name, _] : node_coupling_map[node_data.master_name])
        {
            std::string candidate_name_prefixed = varPrefix_local + candidate_name;
            int new_index = ptr_merged_solver_->get_col_index(candidate_name_prefixed);
            if (new_index == -1)
            {
                std::cerr << LOGLOCATION << "missing variable " << candidate_name << " in " << node_name
                          << " supposedly renamed to " << candidate_name_prefixed << ".";
                ptr_merged_solver_->write_prob_lp(std::filesystem::path(options_.OUTPUTROOT)
                                              / "mergeError.lp");
                ptr_merged_solver_->write_prob_mps(std::filesystem::path(options_.OUTPUTROOT)
                                               / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            candidates_coupling_[candidate_name][node_name] = 
                VariablePositions{
                    .capacity = new_index
                };
            structure_[MasterCouplingConstants::DEFAULT_MASTER_NAME][candidate_name_prefixed] = new_index;
        }

        // Third step : add the subproblem coupling to the merged structure
        for (const auto& [subproblem, positions] : node_coupling_map)
        {
            if (subproblem == node_data.master_name)
                continue;
            std::string subproblem_path = node_data.path / subproblem;
            for (const auto& [candidate_name, position] : positions)
            {
                std::string candidate_name_prefixed = varPrefix_local + candidate_name;
                structure_[subproblem_path][candidate_name_prefixed] = position;
            }
        }
    }

    // Add the delta variables and the constraints that define them
    add_delta_variables();
    logger_->display_message("Delta Variables added successfully");

    add_delta_variables_constraints();
    logger_->display_message("Delta variables constraints added successfully");

    logger_->display_message("Problems merged.");
}

void MergeMasterTrajectoryMPS::add_delta_variables()
{
    // We want to add them efficiently : prepare vectors with all the information needed to modify the solver
    // We want to add two variables per candidate per node
    int delta_variables_count = 2 * candidates_coupling_.size() * tree_.size();

    // Prepare the vectors & reserve space
    std::vector<double> objective_coefs;
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    std::vector<char> col_types;
    std::vector<std::string> col_names;

    objective_coefs.reserve(delta_variables_count);
    lower_bounds.reserve(delta_variables_count);
    upper_bounds.reserve(delta_variables_count);
    col_types.reserve(delta_variables_count);
    col_names.reserve(delta_variables_count); // Not very useful for a dynamic type like string ?

    std::vector<int> mstart_p(delta_variables_count);
    std::iota(mstart_p.begin(), mstart_p.end(), 0);

    int n_var_previous = ptr_merged_solver_->get_ncols();

    // Adding the variables themselves
    for (const auto& node_data : tree_)
    {
        const std::string& node_name = node_data.name;

        std::string node_prefix = make_prefix_from_node(node_name);
        for (auto& [candidate, candidate_data] : candidates_coupling_)
        {   
            std::string var_name_prefix = node_prefix + candidate;
            // dx_plus
            objective_coefs.push_back(0.0);
            lower_bounds.push_back(0);
            upper_bounds.push_back(1e20);
            col_types.push_back('C');
            col_names.push_back(var_name_prefix + "_dx_plus");
            int dx_plus_position = n_var_previous + objective_coefs.size() - 1;
            // dx_minus
            objective_coefs.push_back(0.0);
            lower_bounds.push_back(0);
            upper_bounds.push_back(1e20);
            col_types.push_back('C');
            col_names.push_back(var_name_prefix + "_dx_minus");
            int dx_minus_position = dx_plus_position + 1;

            candidate_data[node_name].dx_plus = dx_plus_position;
            candidate_data[node_name].dx_minus = dx_minus_position;
        }
    }

    // Add to the solver
    solver_addcols(
        *ptr_merged_solver_,
        objective_coefs,
        mstart_p,
        std::vector<int>(0,0),
        std::vector<double>(0, 0.),
        lower_bounds,
        upper_bounds,
        col_types,
        col_names
    );

    return;
}

void MergeMasterTrajectoryMPS::add_delta_variables_constraints(
)
{
    // We will be adding one constraint per candidate per node
    // Each constraint has 4 values in the matrix (welllll not realy but 4 is an upper bound)
    int n_constraints_reserve = candidates_coupling_.size() * tree_.size();
    int n_values_reserve = 4 * (n_constraints_reserve);

    std::vector<int> var_offsets;
    std::vector<int> var_indices;
    std::vector<double> var_values;
    std::vector<double> rhs;
    std::vector<char> constraint_type;

    var_indices.reserve(n_values_reserve);
    var_values.reserve(n_values_reserve);
    rhs.reserve(n_constraints_reserve);
    constraint_type.reserve(n_constraints_reserve);
    var_offsets.reserve(n_constraints_reserve + 1);

    // Add the constraints that define the dx variables.
    for (const auto& node_data : tree_)
    {
        const std::string& node_name = node_data.name;

        if (node_data.parent == std::nullopt)
        {
            for (const auto& [candidate, _] : candidates_coupling_)
            {
                // The constraint is :
                // current::candidate - dx_plus + dx_minus = initial_value
                // Get the initial value if available, use the default value otherwise
                double initial_value = get_candidate_initial_value(candidate);
                logger_->display_message(
                    "Looking up positions for candidate : " + candidate + " -- at node : " + node_name
                );
                const auto& current_candidate_indexes = candidates_coupling_.at(candidate).at(node_name);

                var_offsets.push_back(var_indices.size());
                
                var_indices.push_back(current_candidate_indexes.capacity);
                var_values.push_back(1);
                var_indices.push_back(current_candidate_indexes.dx_plus);
                var_values.push_back(-1);
                var_indices.push_back(current_candidate_indexes.dx_minus);
                var_values.push_back(1);

                rhs.push_back(initial_value);
                constraint_type.push_back('E');
            }
        }
        else
        {
            const std::string& parent_node_name = node_data.parent.value();

            for (const auto& [candidate, _] : candidates_coupling_)
            {
                // The constraint is :
                // current::candidate - parent::candidate - dx_plus + dx_minus = 0

                logger_->display_message(
                    "Looking up positions for candidate : " + candidate + " -- at node : " + node_name
                );
                logger_->display_message(
                    "Looking up positions for candidate : " + candidate + " -- at parent node : " + parent_node_name
                );
                int parent_candidate_index = candidates_coupling_.at(candidate).at(parent_node_name).capacity;
                const auto& current_candidate_indexes = candidates_coupling_.at(candidate).at(node_name);
                
                var_offsets.push_back(var_indices.size());

                var_indices.push_back(current_candidate_indexes.capacity);
                var_values.push_back(1);
                var_indices.push_back(current_candidate_indexes.dx_plus);
                var_values.push_back(-1);
                var_indices.push_back(current_candidate_indexes.dx_minus);
                var_values.push_back(1);
                var_indices.push_back(parent_candidate_index);
                var_values.push_back(-1);

                rhs.push_back(0);
                constraint_type.push_back('E');
            }
        }
    }

    // Add the constraints to the merged problem
    var_offsets.push_back(var_indices.size());
    solver_addrows(*ptr_merged_solver_, constraint_type, rhs, {}, var_offsets, var_indices, var_values);

    return;
}

void MergeMasterTrajectoryMPS::add_coupling_constraints()
{
    // TODO : add the trajectory constraints
    // We need to define them first
    return;
}


/**
 * \brief Merge and solve master and subproblems
 */
void MergeMasterTrajectoryMPS::launch()
{
    build_problem();

    export_problem();
}

