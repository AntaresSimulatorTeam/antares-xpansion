#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"

#include <filesystem>
#include <fmt/format.h>
#include <numeric>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

MergeMasterTrajectoryMPS::CandidateVariableType MergeMasterTrajectoryMPS::parse_variable_type(
  const std::string& s)
{
    using namespace MasterStructureKeys;

    static const std::unordered_map<std::string_view, CandidateVariableType>
      lookup_table{
        {VARIABLE_X, CandidateVariableType::CAPACITY},
        {VARIABLE_DX_PLUS, CandidateVariableType::DX_PLUS},
        {VARIABLE_DX_MINUS, CandidateVariableType::DX_MINUS},
      };

    const auto found = lookup_table.find(s.c_str());
    if (found == lookup_table.end())
    {
        std::cerr << LOGLOCATION
                  << fmt::format("Candidate variable type '{}' should be one of [", s);
        for (const auto& key: lookup_table | std::views::keys)
        {
            std::cerr << key << ", ";
        }
        std::cerr << "]" << std::endl;
        std::exit(1);
    }
    return found->second;
}

char MergeMasterTrajectoryMPS::parse_constraint_type(const std::string& s)
{
    using namespace MasterStructureKeys;

    static const std::unordered_map<std::string_view, char> lookup_table{
      {CONSTRAINT_EQUALS, 'E'},
      {CONSTRAINT_GEQ, 'G'},
      {CONSTRAINT_LEQ, 'L'},
    };

    const auto found = lookup_table.find(s.c_str());
    if (found == lookup_table.end())
    {
        std::cerr << LOGLOCATION << fmt::format("Constraint type '{}' should be one of [", s);
        for (const auto& key: lookup_table | std::views::keys)
        {
            std::cerr << key << ", ";
        }
        std::cerr << "]" << std::endl;
        std::exit(1);
    }
    return found->second;
}

int MergeMasterTrajectoryMPS::VariablePositions::get(CandidateVariableType t) const
{
    switch (t)
    {
    case CandidateVariableType::CAPACITY:
        return capacity;
    case CandidateVariableType::DX_PLUS:
        return dx_plus;
    case CandidateVariableType::DX_MINUS:
        return dx_minus;
    default:
        // Perhaps throw an error ?
        return -1;
    }
}

void MergeMasterTrajectoryMPS::VariablePositions::set(CandidateVariableType t, int i)
{
    switch (t)
    {
    case CandidateVariableType::CAPACITY:
        capacity = i;
        break;
    case CandidateVariableType::DX_PLUS:
        dx_plus = i;
        break;
    case CandidateVariableType::DX_MINUS:
        dx_minus = i;
        break;
    default:
        // Perhaps throw an error ?
        return;
    }
    return;
}

MergeMasterTrajectoryMPS::CandidateCosts::CandidateCosts(const Json::Value& data)
{
    using namespace MasterStructureKeys;
    operation_maintenance = data[KEY_OPERATION_COST].asDouble();
    investment = data[KEY_INVESTMENT_COST].asDouble();
    retirement = data[KEY_RETIREMENT_COST].asDouble();
}

double MergeMasterTrajectoryMPS::CandidateCosts::get(CandidateVariableType t) const
{
    double ret;

    switch (t)
    {
    case CandidateVariableType::CAPACITY:
        ret = operation_maintenance;
        break;
    case CandidateVariableType::DX_PLUS:
        ret = investment;
        break;
    case CandidateVariableType::DX_MINUS:
        ret = retirement;
        break;
    default:
        // Perhaps throw an error ?
        return 0.0;
    }
    return ret;
}

MergeMasterTrajectoryMPS::NodeLpDataLocation::NodeLpDataLocation(const Json::Value& data)
{
    using namespace MasterStructureKeys;
    lp_folder = data[KEY_LP_FOLDER].asString();
    if (data.isMember(KEY_MASTER_MPS_FILE))
    {
        master = data[KEY_MASTER_MPS_FILE].asString();
    }
    if (data.isMember(KEY_STRUCTURE_FILE))
    {
        structure = data[KEY_STRUCTURE_FILE].asString();
    }
}

MergeMasterTrajectoryMPS::TrajectoryConstraint::TrajectoryConstraint(const Json::Value& data)
{
    using namespace MasterStructureKeys;
    // Parse the coefficients
    const auto& coefs_data = data[KEY_COEFFICIENTS];
    for (const auto& candidate_reference: coefs_data.getMemberNames())
    {
        // The name should be a string of format :
        // "node_name::candidate_name::variable_type"
        const auto& split = StringManip::split(candidate_reference, "::");
        if (!(split.size() == 3))
        {
            std::cerr << "Unable to parse the variable reference : " << candidate_reference << "\n"
                      << "Expected format is : node_name::candidate_name::variable_type"
                      << std::endl;
            std::exit(1);
        }
        const auto t = parse_variable_type(split[2]);
        coefficients_map[std::make_tuple(split[0], split[1], t)] = coefs_data[candidate_reference]
                                                                     .asDouble();
    }

    rhs = data[KEY_RHS].asDouble();
    constraint_type = parse_constraint_type(data[KEY_OPERATOR].asString());
}

MergeMasterTrajectoryMPS::TrajectoryGlobalData::TrajectoryGlobalData(const Json::Value& data)
{
    using namespace MasterStructureKeys;

    // Read the initial capacities
    const auto& initial_capacities_data = data[KEY_INITIAL_CAPACITIES];
    // Set a default default value
    initial_capacities[KEY_DEFAULT] = 0;
    for (const auto& candidate_name: initial_capacities_data.getMemberNames())
    {
        initial_capacities[candidate_name] = initial_capacities_data[candidate_name].asDouble();
    }

    // Read the constraints
    const auto& constraints_data = data[KEY_CONSTRAINTS];
    for (const auto& data: constraints_data)
    {
        trajectory_constraints.emplace_back(TrajectoryConstraint(data));
    }
}

MergeMasterTrajectoryMPS::TrajectoryNode::TrajectoryNode(const std::string& node,
                                                         const Json::Value& data):
    name{node}
{
    using namespace MasterStructureKeys;

    if (data.isMember(KEY_PARENT))
    {
        parent = data[KEY_PARENT].asString();

        // Compatibility for root given as hardcoded name
        if (parent == ROOT_NAME)
        {
            parent = std::nullopt;
        }
    }

    // If a MASTER_NAME is given, set it (used when accessing the structure file)
    if (data.isMember(KEY_MASTER_NAME))
    {
        master_name = data[KEY_MASTER_NAME].asString();
    }

    // Pointing each candidate to its associated costs structure
    for (const auto& candidate_name: data[KEY_CANDIDATES].getMemberNames())
    {
        candidates_costs.emplace(
          std::make_pair(candidate_name, CandidateCosts(data[KEY_CANDIDATES][candidate_name])));
    }
}

void MergeMasterTrajectoryMPS::read_tree_structure_file()
{
    using namespace MasterStructureKeys;

    const auto raw_input = get_json_file_content(tree_path_);
    const auto& tree_data = raw_input[KEY_TREE];

    // Read the node by node data
    for (const auto& node_name: tree_data.getMemberNames())
    {
        const auto& node_data = tree_data[node_name];
        tree_.emplace_back(TrajectoryNode(node_name, node_data));
    }

    logger_->display_message("Master coupling map generated successfully.",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);
    logger_->display_message("Number of nodes: " + std::to_string(tree_.size()),
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    // Read the global trajectory data
    trajectory_data_ = TrajectoryGlobalData(raw_input);
}

void MergeMasterTrajectoryMPS::read_node_lp_paths()
{
    const auto raw_input = get_json_file_content(lp_reference_file_filepath_);
    for (const auto& node_name: raw_input.getMemberNames())
    {
        const auto& node_lp_path = raw_input[node_name];
        nodes_lp_paths_.emplace(std::make_pair(node_name, NodeLpDataLocation(node_lp_path)));
    }
}

void MergeMasterTrajectoryMPS::check_nodes_have_lp_folder()
{
    // Every node in the tree must have an associated lp_folder in nodes_lp_paths_
    for (const auto& node: tree_)
    {
        if (!nodes_lp_paths_.contains(node.name))
        {
            std::cerr << "Node '" << node.name
                      << "' must appear in in the list of nodal lp folder.";
        }
    }
}

std::string MergeMasterTrajectoryMPS::make_prefix_from_node(const std::string& node_name) const
{
    return "node_" + node_name + "__";
}

double MergeMasterTrajectoryMPS::get_candidate_initial_value(const std::string& candidate) const
{
    using namespace MasterStructureKeys;

    double initial_value = 0;
    auto it = trajectory_data_.initial_capacities.find(candidate);
    if (it != trajectory_data_.initial_capacities.end())
    {
        initial_value = it->second;
    }
    else
    {
        logger_->display_message("Did not find candidate " + candidate
                                   + " 's initial value, looking up key : '" + KEY_DEFAULT + "'",
                                 LogUtils::LOGLEVEL::INFO,
                                 TRAJECTORY_LOGGER_CONTEXT);
        initial_value = trajectory_data_.initial_capacities.at(KEY_DEFAULT);
    }

    return initial_value;
}

void MergeMasterTrajectoryMPS::build_problem()
{
    logger_->display_message("Inside MergeMasterTrajectoryMPS::build_problem()",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    // Check that the problem format is compatible with the solver
    if (options_.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE
        && StringManip::StringUtils::ToLowercase(options_.SOLVER_NAME) != "xpress")
    {
        std::cerr << LOGLOCATION << "Invalid solver used with the saved file format"
                  << options_.SOLVER_NAME << "\n"
                  << "Can only use Xpress with this option" << std::endl;
        std::exit(1);
    }

    logger_->display_message("Merging master problems...",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    for (const auto& node_data: tree_)
    {
        const std::string& node_name = node_data.name;
        const auto& nodal_lp = nodes_lp_paths_.at(node_name);
        const auto lp_folder = std::filesystem::path(options_.INPUTROOT) / nodal_lp.lp_folder;

        // The master file should not contain the extension, add what it should be based on the mode
        std::string master_file;
        if (options_.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE)
        {
            master_file = nodal_lp.master + SAVE_SUFFIX;
        }
        else if (options_.PROBLEMS_FORMAT == ProblemsFormat::MPS_FILE)
        {
            master_file = nodal_lp.master + MPS_SUFFIX;
        }

        // Read the problem
        logger_->display_message("Reading problem " + (lp_folder / nodal_lp.master).string(),
                                 LogUtils::LOGLEVEL::INFO,
                                 TRAJECTORY_LOGGER_CONTEXT);

        SolverAbstract::Ptr solver_local = get_local_solver(lp_folder, master_file);
        solver_local->set_output_log_level(options_.LOG_LEVEL);

        StandardLp lpData(*solver_local);
        std::string varPrefix_local = make_prefix_from_node(node_name);

        // Perhaps we could think of a way to include / use AbstractMergeMPS::merge_local_solver
        // But it does not do exactly what we do here for now, particularily when building the new
        // structure file It returns a map : old_var_name -> new_position Where as we are building :
        // master -> prefixed_var_name -> new_position And : subproblem -> prefixed_var_name ->
        // unchanged_position
        lpData.append_in(*ptr_merged_solver_, varPrefix_local);

        // Load the coupling map (structure file) for this node
        // It will be used to iterate through the investment candidates in the node and get their
        // position in the merged problem
        CouplingMap node_coupling_map = CouplingMapGenerator::BuildInput(lp_folder
                                                                           / nodal_lp.structure,
                                                                         logger_.get());

        // Second step : get the candidate's position in the merged problem
        for (const auto& [candidate_name, _]: node_coupling_map[node_data.master_name])
        {
            const std::string candidate_name_prefixed = varPrefix_local + candidate_name;
            int new_index = ptr_merged_solver_->get_col_index(candidate_name_prefixed);
            if (new_index == -1)
            {
                terminate_on_missing_variable(node_name, candidate_name, candidate_name_prefixed);
            }
            // Create the VariablePositions entry for this candidate
            candidates_coupling_[candidate_name][node_name].set(CAPACITY, new_index);
            structure_[MasterStructureKeys::DEFAULT_MASTER_NAME][candidate_name_prefixed]
              = new_index;
        }

        // Third step : add the subproblem coupling to the merged structure
        for (const auto& [subproblem, positions]: node_coupling_map)
        {
            if (subproblem == node_data.master_name)
            {
                continue;
            }

            const std::string subproblem_path = (nodal_lp.lp_folder / subproblem).string();
            for (const auto& [candidate_name, position]: positions)
            {
                const std::string candidate_name_prefixed = varPrefix_local + candidate_name;
                structure_[subproblem_path][candidate_name_prefixed] = position;
            }
        }
    }

    const std::filesystem::path structure_file = std::filesystem::path(options_.OUTPUTROOT)
                                                 / "structure.txt";
    export_structure_file(structure_file, structure_);

    // Add the delta variables and the constraints that define them
    add_delta_variables();
    logger_->display_message("Delta Variables added successfully",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    add_delta_variables_constraints();
    logger_->display_message("Delta variables constraints added successfully",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    set_objective_from_data();
    logger_->display_message("Successfully set the objective according to the data",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);

    add_coupling_constraints();
    logger_->display_message("Successfully added the trajectory constraints",
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);
}

void MergeMasterTrajectoryMPS::add_delta_variables()
{
    // We want to add them efficiently : prepare vectors with all the information needed to modify
    // the solver We want to add two variables per candidate per node
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
    for (const auto& node_data: tree_)
    {
        const std::string& node_name = node_data.name;

        std::string node_prefix = make_prefix_from_node(node_name);
        for (auto& [candidate, candidate_data]: candidates_coupling_)
        {
            std::string var_name_prefix = node_prefix + candidate;

            // dx_plus
            objective_coefs.push_back(0.0);
            lower_bounds.push_back(0);
            upper_bounds.push_back(1e20); // TODO Unbound instead of hard coded
            col_types.push_back('C');
            col_names.push_back(var_name_prefix + "_dx_plus");
            int dx_plus_position = n_var_previous + objective_coefs.size() - 1;

            // dx_minus
            objective_coefs.push_back(0.0);
            lower_bounds.push_back(0);
            upper_bounds.push_back(1e20); // TODO Unbound instead of hard coded
            col_types.push_back('C');
            col_names.push_back(var_name_prefix + "_dx_minus");
            int dx_minus_position = dx_plus_position + 1;

            candidate_data[node_name].set(DX_PLUS, dx_plus_position);
            candidate_data[node_name].set(DX_MINUS, dx_minus_position);
        }
    }

    // Add to the solver
    solver_addcols(*ptr_merged_solver_,
                   objective_coefs,
                   mstart_p,
                   std::vector<int>(0, 0),
                   std::vector<double>(0, 0.),
                   lower_bounds,
                   upper_bounds,
                   col_types,
                   col_names);

    return;
}

void MergeMasterTrajectoryMPS::add_delta_variables_constraints()
{
    // We will be adding one constraint per candidate per node
    // Each constraint has 4 values in the matrix (well not really but 4 is an upper bound)
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
    for (const auto& node_data: tree_)
    {
        const std::string& node_name = node_data.name;

        if (!node_data.parent.has_value())
        {
            for (const auto& [candidate, _]: candidates_coupling_)
            {
                // The constraint is :
                // current::candidate - dx_plus + dx_minus = initial_value
                // Get the initial value if available, use the default value otherwise
                double initial_value = get_candidate_initial_value(candidate);
                const auto& current_candidate_indexes = candidates_coupling_.at(candidate).at(
                  node_name);

                var_offsets.push_back(var_indices.size());

                var_indices.push_back(current_candidate_indexes.get(CAPACITY));
                var_values.push_back(1);

                var_indices.push_back(current_candidate_indexes.get(DX_PLUS));
                var_values.push_back(-1);

                var_indices.push_back(current_candidate_indexes.get(DX_MINUS));
                var_values.push_back(1);

                rhs.push_back(initial_value);
                constraint_type.push_back('E');
            }
        }
        else [[likely]]
        {
            const std::string& parent_node_name = node_data.parent.value();

            for (const auto& [candidate, _]: candidates_coupling_)
            {
                // The constraint is :
                // current::candidate - parent::candidate - dx_plus + dx_minus = 0
                const auto& parent_candidate_indexes = candidates_coupling_.at(candidate).at(
                  parent_node_name);
                const auto& current_candidate_indexes = candidates_coupling_.at(candidate).at(
                  node_name);

                var_offsets.push_back(var_indices.size());

                var_indices.push_back(current_candidate_indexes.get(CAPACITY));
                var_values.push_back(1);

                var_indices.push_back(current_candidate_indexes.get(DX_PLUS));
                var_values.push_back(-1);

                var_indices.push_back(current_candidate_indexes.get(DX_MINUS));
                var_values.push_back(1);

                var_indices.push_back(parent_candidate_indexes.get(CAPACITY));
                var_values.push_back(-1);

                rhs.push_back(0);
                constraint_type.push_back('E');
            }
        }
    }

    // Add the constraints to the merged problem
    var_offsets.push_back(var_indices.size());
    solver_addrows(*ptr_merged_solver_,
                   constraint_type,
                   rhs,
                   {},
                   var_offsets,
                   var_indices,
                   var_values);

    return;
}

void MergeMasterTrajectoryMPS::set_objective_from_data()
{
    std::vector<int> indexes;
    std::vector<double> coefficients;

    // Pre reserve the size : 3 variables per node per candidate
    int nb_coefficients_reserve = 3 * tree_.size() * candidates_coupling_.size();
    indexes.reserve(nb_coefficients_reserve);
    coefficients.reserve(nb_coefficients_reserve);

    for (const auto& node: tree_)
    {
        for (const auto& [candidate, positions_per_node]: candidates_coupling_)
        {
            const auto& costs = node.candidates_costs.at(candidate);
            const auto& positions = positions_per_node.at(node.name);

            // To be discussed : node weights & discounting
            indexes.push_back(positions.get(CAPACITY));
            coefficients.push_back(costs.get(CAPACITY));

            indexes.push_back(positions.get(DX_PLUS));
            coefficients.push_back(costs.get(DX_PLUS));

            indexes.push_back(positions.get(DX_MINUS));
            coefficients.push_back(costs.get(DX_MINUS));
        }
    }

    ptr_merged_solver_->chg_obj(indexes, coefficients);
}

void MergeMasterTrajectoryMPS::add_coupling_constraints()
{
    // Prepare the vectors & reserve the adequate size.
    const auto& constraints = trajectory_data_.trajectory_constraints;

    int n_constraints_reserve = constraints.size();
    int n_values_reserve(0);
    for (const auto& cons: constraints)
    {
        n_values_reserve += cons.coefficients_map.size();
    }

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

    for (const auto& cons: constraints)
    {
        var_offsets.push_back(var_indices.size());

        for (const auto& [var_ref, val]: cons.coefficients_map)
        {
            const std::string& node_name = std::get<0>(var_ref);
            const std::string& candidate = std::get<1>(var_ref);
            const CandidateVariableType& var_type = std::get<2>(var_ref);
            const int index = candidates_coupling_.at(candidate).at(node_name).get(var_type);

            var_indices.push_back(index);
            var_values.push_back(val);
        }

        rhs.push_back(cons.rhs);
        constraint_type.push_back(cons.constraint_type);
    }

    var_offsets.push_back(var_indices.size());

    solver_addrows(*ptr_merged_solver_,
                   constraint_type,
                   rhs,
                   {},
                   var_offsets,
                   var_indices,
                   var_values);

    return;
}

/**
 * \brief Merge and solve master and subproblems
 */
void MergeMasterTrajectoryMPS::launch()
{
    logger_->display_message("Parsing structure file at " + tree_path_.string(),
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);
    read_tree_structure_file();

    logger_->display_message("Parsing nodal lp folder data at "
                               + lp_reference_file_filepath_.string(),
                             LogUtils::LOGLEVEL::INFO,
                             TRAJECTORY_LOGGER_CONTEXT);
    read_node_lp_paths();

    build_problem();

    // TODO To be changed
    export_problem("log_merged", true);
}
