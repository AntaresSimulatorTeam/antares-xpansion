#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/merge_master_mps/StandardLp.h"
#include "antares-xpansion/benders/merge_master_mps/MasterCoupling.h"

#include <filesystem>
#include <utility>
#include <numeric>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
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
    

void MergeMasterMPS::launch()
{
    _logger->display_message("Inside MergeMasterMPS::launch()");
    const auto inputRootDir = std::filesystem::path(_options.INPUTROOT);
    auto structure_path(inputRootDir / _options.STRUCTURE_FILE);
    MasterCouplingMap master_coupling = MasterCouplingMapGenerator::BuildInput(structure_path, _logger.get());

    SolverFactory factory;
    SolverAbstract::Ptr mergedSolver_l = factory.create_solver(_options.SOLVER_TO_USE);
    mergedSolver_l->set_output_log_level(_options.LOG_LEVEL);

    // This coupling map contains, for each candidate, for each node it appears in,
    // where the candidate is located in the merged master problem
    // For this to work, the name of the candidates have to be the same in all problems
    CouplingMap capacity_variable_coupling;
    // int cntProblems_l(0);

    _logger->display_message("Merging master problems...");

    for (const auto& [node_name, node_data] : master_coupling)
    {

        auto problem_file = std::filesystem::path(_options.INPUTROOT) / node_data.lp_folder / node_data.master_mps_file;
        SolverAbstract::Ptr solver_l = factory.create_solver(_options.SOLVER_TO_USE);
        solver_l->set_output_log_level(_options.LOG_LEVEL);

        // Read the problem
        solver_l->read_prob_mps(problem_file);

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

        // First step : get the candidate's position in the merged problem
        // The coupling map must contain the key "master"
        for (const auto& [candidate_name, _] : node_coupling_map["master"])
        {
            int new_index = mergedSolver_l->get_col_index(varPrefix_l + candidate_name);
            if (new_index == -1)
            {
                std::cerr << LOGLOCATION << "missing variable " << candidate_name << " in " << node_name
                          << " supposedly renamed to " << varPrefix_l + candidate_name << ".";
                mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT)
                                              / "mergeError.lp");
                mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                               / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            capacity_variable_coupling[candidate_name][node_name] = new_index;
        }

        // Second step : rename the variables in the subproblems (probably not necessary)
        // Do stuff
    }
    // Next : add the trajectory constraints linking the nodes
    // TODO : determine the input format for those constraints
    // Do stuff

    // Finally, write the new structure file to the output directory
    // Do stuff

    _logger->display_message("Problems merged.");
    _logger->display_message("Writing mps file");
    mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
                                   / ("log_merged" + MPS_SUFFIX));
    _logger->display_message("Writing lp file");
    mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT) / "log_merged.lp");

}
