#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/merge_master_mps/MasterCoupling.h"


#include <filesystem>
#include <utility>
#include <numeric>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

MergeMasterMPS::MergeMasterMPS(MergeMasterMPSOptions options,
                   Logger logger,
                   std::shared_ptr<Output::OutputWriter> writer):
    _options(std::move(options)),
    _logger(std::move(logger)),
    _writer(std::move(writer))
{
}


/*!
 *  \brief Multiply the objective function by a weight factor
 *
 *  \param solver : solver to modify
 *
 *  \param weight : weight factor to apply
 */
void multiply_objective_by_weight_factor(
    SolverAbstract::Ptr& solver,
    double weight)
{
    int ncols = solver->get_ncols();
    std::vector<double> obj_coef(ncols);
    // Sequence vector : indexes are 0 to ncols - 1
    std::vector<int> sequence(ncols);
    std::iota(sequence.begin(), sequence.end(), 0);
    // Multply the objective function by the weight factor
    solver_get_obj_func_coeffs(*solver, obj_coef, 0, ncols - 1);
    for (auto& c: obj_coef)
    {
        c *= weight;
    }
    // Set the new objective function
    solver->chg_obj(sequence, obj_coef);
    
    return;
}


void MergeMasterMPS::addTrajectoryConstraints(
    SolverAbstract& merged_solver,
    const MasterCouplingMap& master_coupling,
    const TrajectoryGlobalData& trajectory_data,
    const CouplingMap& candidates_coupling
)
{
    // First step is counting the number of constraints we want to add
    // and the number of values we will add to the sparse matrix
    size_t n_constraints_reserve = 0;
    size_t n_values_reserve = 0;
    for (const auto& [node_name, node_data] : master_coupling)
    {
        const auto& candidates_constraints = node_data.candidate_constraints;
        // We add two constraints per candidate (min and max investment at this node)
        // This is hardcoded for now :(
        if (node_data.parent != MasterCouplingConstants::ROOT_NAME)
        {
            n_constraints_reserve += 2 * candidates_constraints.size();
            n_values_reserve += 2 * 2 * candidates_constraints.size();
        } else {
            // We add one constraint per candidate (max investment at the first date),
            // and it only has one value per candidate (only one variable involved)
            n_constraints_reserve += candidates_constraints.size();
            n_values_reserve += candidates_constraints.size();
        }
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

    // Second step : now that we have a merged master problem, we add the trajectory constraints
    for (const auto& [node_name, node_data] : master_coupling)
    {
        std::string parent_node_name = node_data.parent;

        const auto& candidates_constraints = node_data.candidate_constraints;
        for (const auto& [candidate, constraint_data] : candidates_constraints)
        {
            if (parent_node_name == MasterCouplingConstants::ROOT_NAME) {
                // The constraint is :
                // current::candidate <= max_investment + initial_value
                // Get the initial value if available, use the default value otherwise
                double initial_value = 0;
                auto it = trajectory_data.initial_capacities.find(candidate);
                if (it != trajectory_data.initial_capacities.end())
                {
                    initial_value = it->second;
                }
                else
                {
                    initial_value = trajectory_data.initial_capacities.at(MasterCouplingConstants::KEY_DEFAULT);
                }
                _logger->display_message("Initial value for candidate " + candidate + " : " + std::to_string(initial_value));
                // Max investment
                int current_candidate_index = candidates_coupling.at(candidate).at(node_name);

                var_offsets.push_back(var_indices.size());
                var_indices.push_back(current_candidate_index);
                var_values.push_back(1);

                rhs.push_back(constraint_data.max_investment + initial_value);
                constraint_type.push_back('L');

            }
            else
            {
                // The constraints are :
                // current::candidate - parent::candidate <= max_investment
                // current::candidate - parent::candidate >= min_investment
                int parent_candidate_index = candidates_coupling.at(candidate).at(parent_node_name);
                int current_candidate_index = candidates_coupling.at(candidate).at(node_name);
                
                // Max investment 
                var_offsets.push_back(var_indices.size());

                var_indices.push_back(current_candidate_index);
                var_values.push_back(1);

                var_indices.push_back(parent_candidate_index);
                var_values.push_back(-1);

                rhs.push_back(constraint_data.max_investment);
                constraint_type.push_back('L');

                // Min investment
                var_offsets.push_back(var_indices.size());

                var_indices.push_back(current_candidate_index);
                var_values.push_back(1);

                var_indices.push_back(parent_candidate_index);
                var_values.push_back(-1);

                rhs.push_back(constraint_data.min_investment);
                constraint_type.push_back('G');
            }
        }
    }

    // Add the constraints to the merged problem
    var_offsets.push_back(var_indices.size());
    solver_addrows(merged_solver, constraint_type, rhs, {}, var_offsets, var_indices, var_values);

    return;
}
    

void MergeMasterMPS::launch()
{
    _logger->display_message("Inside MergeMasterMPS::launch()");

    const auto inputRootDir = std::filesystem::path(_options.INPUTROOT);
    auto structure_path(inputRootDir / _options.STRUCTURE_FILE);
    _logger->display_message("Trying to parse strucutre file at " + std::string(structure_path));

    const auto& [trajectory_data, master_coupling] = 
        MasterCouplingMapGenerator::BuildInput(structure_path, _logger.get());

    // Check that the problem format is compatible with the solver
    if(_options.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE
        && _options.SOLVER_NAME != "Xpress")
    {
        std::cerr << LOGLOCATION <<
            "Invalid solver used with the saved file format" << _options.SOLVER_NAME << "\n" <<
            "Can only use Xpress with this option" << std::endl;
    }

    SolverFactory factory;
    SolverAbstract::Ptr mergedSolver_l = factory.create_solver(_options.SOLVER_NAME);
    mergedSolver_l->set_output_log_level(_options.LOG_LEVEL);

    // We want to verify the studies as we go along : we check that the candidates are the same at every node
    std::set<std::string> candidate_set;

    // TODO : verify depth : we want every node at a given depth to represent the same investmen time point

    // TODO : Perhaps move the verifications out of the launch function ?
    // This mean more readable code : but more time consumming as we load the coupling map twice,
    // once for the verification and once for the merging

    // This coupling map contains, for each candidate, for each node it appears in,
    // where the candidate is located in the merged master problem
    // For this to work, the name of the candidates have to be the same in all problems
    CouplingMap capacity_variable_coupling;
    // int cntProblems_l(0);

    // We also need to need to generate a merged_structure.txt file
    // For the master problem : all the renamed variables and their index in the merged problem
    // For each of the node's subproblems : the renamed variables and their index in the respective subproblem files
    CouplingMap merged_structure;

    _logger->display_message("Merging master problems...");

    for (const auto& [node_name, node_data] : master_coupling)
    {

        auto problem_file = std::filesystem::path(_options.INPUTROOT) / node_data.lp_folder / node_data.master_mps_file;
        SolverAbstract::Ptr solver_l = factory.create_solver(_options.SOLVER_NAME);
        solver_l->set_output_log_level(_options.LOG_LEVEL);

        // Read the problem
        _logger->display_message("Reading problem " + problem_file.string());

        if(_options.PROBLEMS_FORMAT == ProblemsFormat::MPS_FILE)
        {
            _logger->display_message("Reading under the MPS format");
            solver_l->read_prob_mps(problem_file);
        } 
        else if (_options.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE)
        {
            _logger->display_message("Reading a saved file format");
            solver_l->restore_prob(problem_file);
        }    

        // Multiply the objective function by the weight factor
        double weight_factor = node_data.weight_factor;
        _logger->display_message("Weight factor for node " + node_name + " : " + std::to_string(weight_factor));
        multiply_objective_by_weight_factor(solver_l, weight_factor);

        StandardLp lpData(*solver_l);
        std::string varPrefix_l = "node_" + node_name + "__";

        lpData.append_in(mergedSolver_l, varPrefix_l);

        // Load the coupling map (structure file) for this node
        // It will be used for
        // 1. To iterate through the investment candidates in the node and get their position in the merged problem
        // 2. To rename the corresponding variables in the node's subproblems the same way they are renamed in the merged problem
        // (To check, but the renaming might not even be necessary)
        CouplingMap node_coupling_map = CouplingMapGenerator::BuildInput(
            std::filesystem::path(_options.INPUTROOT) / node_data.lp_folder / node_data.structure_file,
            _logger.get()
        );

        // First step : verification of the candidates
        // Note : this is inneficient as we do a lot of copies, perhaps we could make it better ?
        std::set<std::string> node_candidate_set;
        for (const auto& [candidate_name, _] : node_coupling_map[node_data.master_name])
        {
            node_candidate_set.insert(candidate_name);
        }
        if (candidate_set.empty())
        {
            candidate_set = node_candidate_set;
        }
        else
        {
            // Check that the candidates are the same in all problems
            if (node_candidate_set != candidate_set)
            {
                std::cerr << LOGLOCATION << "The candidates are not the same in all nodes.";
                std::exit(1);
            }
        }

        // Second step : get the candidate's position in the merged problem
        // The coupling map must contain the key given in the master_name of the node's data
        // By default if not given, we assume the name to be "master"
        for (const auto& [candidate_name, _] : node_coupling_map[node_data.master_name])
        {
            std::string candidate_name_prefixed = varPrefix_l + candidate_name;
            int new_index = mergedSolver_l->get_col_index(candidate_name_prefixed);
            if (new_index == -1)
            {
                std::cerr << LOGLOCATION << "missing variable " << candidate_name << " in " << node_name
                          << " supposedly renamed to " << candidate_name_prefixed << ".";
                mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT)
                                              / "mergeError.lp");
                mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                               / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            capacity_variable_coupling[candidate_name][node_name] = new_index;
            merged_structure[MasterCouplingConstants::DEFAULT_MASTER_NAME][candidate_name_prefixed] = new_index;
        }

        // Third step : add the subproblem coupling to the merged structure
        for (const auto& [subproblem, positions] : node_coupling_map)
        {
            if (subproblem == node_data.master_name)
                continue;
            std::string subproblem_prefixed = varPrefix_l + subproblem;
            for (const auto& [candidate_name, position] : positions)
            {
                std::string candidate_name_prefixed = varPrefix_l + candidate_name;
                merged_structure[subproblem_prefixed][candidate_name_prefixed] = position;
            }
        }
    }

    // Next : add the trajectory constraints linking the nodes

    addTrajectoryConstraints(
        *mergedSolver_l,
        master_coupling,
        trajectory_data,
        capacity_variable_coupling
    );

    // Finally, write the new structure file to the output directory
    std::string output_structure_file = _options.OUTPUTROOT + "/structure.txt";
    CouplingMapGenerator::WriteCouplingMap(
        merged_structure,
        std::filesystem::path(output_structure_file),
        _logger.get()
    );

    _logger->display_message("Problems merged.");
    _logger->display_message("Writing mps file");
    mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                   / ("log_merged" + MPS_SUFFIX));
    _logger->display_message("Writing lp file");
    mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT) / "log_merged.lp");

}
