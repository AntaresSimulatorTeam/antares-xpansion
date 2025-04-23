#include "antares-xpansion/benders/merge_mps/MergeMPS.h"

#include <filesystem>
#include <numeric>
#include <ranges>
#include <utility>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/helpers/Timer.h"

AbstractMergeMPS::AbstractMergeMPS(MergeMPSOptions options,
                                   Logger logger,
                                   std::shared_ptr<Output::OutputWriter> writer):
    writer_(std::move(writer)),
    options_(std::move(options)),
    logger_(std::move(logger))
{
    if (options_.SOLVER_NAME == "COIN")
    {
        options_.SOLVER_NAME = "CBC";
    }

    SolverFactory factory;
    ptr_merged_solver_ = factory.create_solver(options_.SOLVER_NAME);

    ptr_merged_solver_->set_output_log_level(options_.LOG_LEVEL);
}


/**
* \brief Creates a local solver from a MPS file
*
* \param root_dir : Directory of MPS file
*
* \param filename : MPS file name
*/
SolverAbstract::Ptr AbstractMergeMPS::get_local_solver(const std::filesystem::path& root_dir,
                                                      const std::string& filename) const
{
   /**
    * Limitation: on windows may not support master problem with full path as name
    */
   SolverAbstract::Ptr ptr_solver = factory_.create_solver(options_.SOLVER_NAME);
   ptr_solver->set_output_log_level(options_.LOG_LEVEL);
   ptr_solver->read_prob_mps(root_dir / filename);
   return ptr_solver;
}


/**
 * \brief Weights local solver's objective function by a given value
 *
 * \param local_solver : Subproblem solver that will be modified
 *
 * \param weight : Factor to apply
 */
void AbstractMergeMPS::multiply_obj_by_weight_factor(SolverAbstract& local_solver,
                                                     double weight) const
{
    const int nb_cols{local_solver.get_ncols()};

    std::vector<int> indices(nb_cols);
    std::iota(indices.begin(), indices.end(), 0);

    std::vector<double> obj_coeff(nb_cols);
    solver_get_obj_func_coeffs(local_solver, obj_coeff, 0, nb_cols - 1);

    std::for_each(obj_coeff.begin(),
                  obj_coeff.end(),
                  [weight](double& coeff) { return coeff *= weight; });

    local_solver.chg_obj(indices, obj_coeff);
}

/**
 * \brief Export problem into mps and lp files
 */
void AbstractMergeMPS::export_problem()
{
    const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
    logger_->display_message("Problems merged.");
    logger_->display_message("Writing mps file");
    ptr_merged_solver_->write_prob_mps(output_root / ("log_merged" + MPS_SUFFIX));
    logger_->display_message("Writing lp file");
    ptr_merged_solver_->write_prob_lp(output_root / "log_merged.lp");
}




/**
 * \brief Build merged problem
 */
void MergeMasterSubproblemMPS::build_problem()
{
    const auto input_root_dir = std::filesystem::path(options_.INPUTROOT);
    const auto structure_path(input_root_dir / options_.STRUCTURE_FILE);

    structure_ = CouplingMapGenerator::BuildInput(structure_path, logger_.get(), "Merge mps");

    // TODO Investigate why following check
    // TODO creates a segfault when structure.txt is empty
    // if (structure_.empty())
    // {
    //     logger_->display_message("Nothing to merge. Returning empty problem.");
    //     return;
    // }

    const int nb_sub_problems = structure_.size() - 1;
    const auto root_dir = std::filesystem::path(options_.INPUTROOT);
    SolverFactory factory;

    logger_->display_message("Merging problems...");

    int current_prob_id{0};
    for (auto& [filename, var_map]: structure_)
    {
        SolverAbstract::Ptr ptr_solver = factory.create_solver(options_.SOLVER_NAME);

        ptr_solver->set_output_log_level(options_.LOG_LEVEL);

        ptr_solver->read_prob_mps(root_dir / filename);

        // Separate Master and Subproblems by a specific name ID
        // given in the options file
        if (filename != options_.MASTER_NAME)
        {
            // Change the weight of coeff in the objective function
            // The strategy is defined in the input options
            const double weight = get_problem_obj_weight(nb_sub_problems, filename);
            multiply_obj_by_weight_factor(*ptr_solver, weight);
        }

        StandardLp lpData(*ptr_solver);
        const std::string var_prefix = "prob" + std::to_string(current_prob_id++) + "_";

        // Prefix the name of the problem (Master and slaves alike)
        // along with the counting
        lpData.append_in(*ptr_merged_solver_, var_prefix);

        for (auto& [var_name, var_idx]: var_map)
        {
            const int merged_col_index = ptr_merged_solver_->get_col_index(var_prefix + var_name);
            if (merged_col_index == -1)
            {
                const auto output_root = std::filesystem::path(options_.OUTPUTROOT);
                std::cerr << LOGLOCATION << "missing variable " << var_name << " in " << filename
                          << " supposedly renamed to " << var_prefix + var_name << ".";
                ptr_merged_solver_->write_prob_lp(output_root / "mergeError.lp");
                ptr_merged_solver_->write_prob_mps(output_root / ("mergeError" + MPS_SUFFIX));
                std::exit(1);
            }
            // TODO Not yet happy with this part
            // TODO Think of a better data struct maybe?
            var_idx = merged_col_index;
        }
    }

    add_coupling_constraints();
}


/**
 * \brief Add coupling equality constraints between subproblems
 */
void MergeMasterSubproblemMPS::add_coupling_constraints()
{
    std::map<std::string, std::vector<int>> variables;
    for (const auto& [_, var_map]: structure_)
    {
        for (const auto& [var_name, var_idx]: var_map)
        {
            variables[var_name].push_back(var_idx);
        }
    }

    // Add n-1 new constraints per variable where n is
    // the number of problems where this variable appears
    // i.e. the number of columns in the merged problem
    // representing this variable
    const size_t nb_rows_reserve = std::accumulate(variables.cbegin(),
                                                   variables.cend(),
                                                   size_t{0},
                                                   [](size_t acc, const auto& pair)
                                                   {
                                                       const auto& indices = pair.second;
                                                       return acc + indices.size() - 1;
                                                   });
    const size_t nb_elem_reserve = 2 * nb_rows_reserve;

    logger_->display_message("About to add " + std::to_string(nb_rows_reserve)
                             + " coupling constraints");

    std::vector<int> mstart; // Constraints' offsets
    mstart.reserve(nb_rows_reserve + 1);

    std::vector<int> mclind;     // Variables' indices
    std::vector<double> dmatval; // Variables' values
    dmatval.reserve(nb_elem_reserve);
    mclind.reserve(nb_elem_reserve);

    int nb_rows{0};
    int nb_elem{0};
    for (const auto& [var_name, indices]: variables)
    {
        const int ref_var_idx = indices[0];

        // Starting from second element onwards
        // Add one equality constraint per pair of variables:
        // 1 * ref - 1 * second = 0
        for (int var_idx: indices | std::views::drop(1))
        {
            mstart.push_back(nb_elem);

            mclind.push_back(ref_var_idx);
            dmatval.push_back(1);
            ++nb_elem;

            mclind.push_back(var_idx);
            dmatval.push_back(-1);
            ++nb_elem;

            ++nb_rows;
        }

        logger_->display_message(var_name + " : " + std::to_string(indices.size() - 1)
                                 + " coupling constraints built");
    }
    mstart.push_back(nb_elem);

    std::vector<double> rhs(nb_rows, 0);    // Constraints' rhs
    std::vector<char> qrtype(nb_rows, 'E'); // Constraints' types

    solver_addrows(*ptr_merged_solver_, qrtype, rhs, {}, mstart, mclind, dmatval);
}
