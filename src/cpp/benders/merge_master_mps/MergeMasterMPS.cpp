#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/merge_master_mps/MasterCoupling.h"


#include <filesystem>
#include <utility>
#include <numeric>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"


std::string MergeMasterTrajectoryMPS::make_prefix_from_node(const std::string& node_name) const
{
    return "node_" + node_name + "__";
}

double MergeMasterTrajectoryMPS::get_candidate_initial_value(const std::string& candidate) const
{
    double initial_value = 0;
    auto it = trajectory_data_.initial_capacities.find(candidate);
    if (it != trajectory_data_.initial_capacities.end())
    {
        initial_value = it->second;
    }
    else
    {
        initial_value = trajectory_data_.initial_capacities.at(MasterCouplingConstants::KEY_DEFAULT);
    }

    return initial_value;
}

void MergeMasterTrajectoryMPS::build_problem()
{
    logger_->display_message("Inside MergeMasterTrajectoryMPS::build()");

    // Loading the data, could be seperated into a load() function
    const auto inputRootDir = std::filesystem::path(options_.INPUTROOT);
    auto structure_path(inputRootDir / options_.STRUCTURE_FILE);
    logger_->display_message("Trying to parse structure file at " + std::string(structure_path));

    const auto& data_pair = 
        MasterCouplingMapGenerator::BuildInput(structure_path, logger_.get());
    // Unecessary copy ?
    trajectory_data_ = data_pair.first;
    master_coupling_ = data_pair.second;

    // Check that the problem format is compatible with the solver
    if(options_.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE
        && options_.SOLVER_NAME != "Xpress")
    {
        std::cerr << LOGLOCATION <<
            "Invalid solver used with the saved file format" << options_.SOLVER_NAME << "\n" <<
            "Can only use Xpress with this option" << std::endl;
    }

    SolverFactory factory;

    logger_->display_message("Merging master problems...");

    for (const auto& [node_name, node_data] : master_coupling_)
    {

        auto problem_file = std::filesystem::path(options_.INPUTROOT) / node_data.lp_folder / node_data.master_mps_file;
        SolverAbstract::Ptr solver_local = factory.create_solver(options_.SOLVER_NAME);
        solver_local->set_output_log_level(options_.LOG_LEVEL);

        // Read the problem
        logger_->display_message("Reading problem " + problem_file.string());

        if(options_.PROBLEMS_FORMAT == ProblemsFormat::MPS_FILE)
        {
            logger_->display_message("Reading under the MPS format");
            solver_local->read_prob_mps(problem_file);
        } 
        else if (options_.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE)
        {
            logger_->display_message("Reading a saved file format");
            solver_local->restore_prob(problem_file);
        }    

        // Multiply the objective function by the weight factor
        double weight_factor = node_data.weight_factor;
        logger_->display_message("Weight factor for node " + node_name + " : " + std::to_string(weight_factor));
        AbstractMergeMPS::multiply_obj_by_weight_factor(*solver_local, weight_factor);

        StandardLp lpData(*solver_local);
        std::string varPrefix_local = make_prefix_from_node(node_name);

        lpData.append_in(*ptr_merged_solver_, varPrefix_local);

        // Load the coupling map (structure file) for this node
        // It will be used to iterate through the investment candidates in the node and get their position in the merged problem
        CouplingMap node_coupling_map = CouplingMapGenerator::BuildInput(
            std::filesystem::path(options_.INPUTROOT) / node_data.lp_folder / node_data.structure_file,
            logger_.get()
        );

        // Second step : get the candidate's position in the merged problem
        // The coupling map must contain the key given in the master_name of the node's data
        // By default if not given, we assume the name to be "master"
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
            std::string subproblem_path = std::filesystem::path(node_data.lp_folder)
                / subproblem;
            for (const auto& [candidate_name, position] : positions)
            {
                std::string candidate_name_prefixed = varPrefix_local + candidate_name;
                structure_[subproblem_path][candidate_name_prefixed] = position;
            }
        }
    }

    logger_->display_message("After inital merging : master_coupling_ has size : " + std::to_string(master_coupling_.size()));
    logger_->display_message("candidates_coupling has size : " + std::to_string(candidates_coupling_.size()));

    // Add the delta variables and the constraints that define them
    add_delta_variables();

    add_delta_variables_constraints();

    logger_->display_message("Problems merged.");
}

void MergeMasterTrajectoryMPS::add_delta_variables()
{
    // We want to add them efficiently : prepare vectors with all the information needed to modify the solver
    // We want to add two variables per candidate per node
    int delta_variables_count = 2 * candidates_coupling_.size() * master_coupling_.size();

    // Prepare the vectors
    std::vector<double> objective_coefs;
    std::vector<double> lower_bounds;
    std::vector<double> upper_bounds;
    std::vector<char> col_types;
    std::vector<std::string> col_names;

    // Reserve space
    objective_coefs.reserve(delta_variables_count);
    lower_bounds.reserve(delta_variables_count);
    upper_bounds.reserve(delta_variables_count);
    col_types.reserve(delta_variables_count);
    col_names.reserve(delta_variables_count); // Not very pertinent for a dynamic type like string ?

    std::vector<int> mstart_p(delta_variables_count);
    std::iota(mstart_p.begin(), mstart_p.end(), 0);

    int n_var_previous = ptr_merged_solver_->get_ncols();

    // First part : adding the variables themselves
    for (const auto& [node_name, _] : master_coupling_)
    {
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
    int n_constraints_reserve = candidates_coupling_.size() * master_coupling_.size();
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

    // Second part : add the constraints that define the dx variables.

    for (const auto& [node_name, node_data] : master_coupling_)
    {
        std::string parent_node_name = node_data.parent;

        for (const auto& [candidate, _] : candidates_coupling_)
        {
            if (parent_node_name == MasterCouplingConstants::ROOT_NAME) {
                // The constraint is :
                // current::candidate - dx_plus + dx_minus = initial_value
                // Get the initial value if available, use the default value otherwise
                double initial_value = get_candidate_initial_value(candidate);
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
            else
            {
                // The constraint is :
                // current::candidate - parent::candidate - dx_plus + dx_minus = 0
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
    return;
}