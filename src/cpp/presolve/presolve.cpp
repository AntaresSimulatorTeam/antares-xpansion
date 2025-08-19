#include "antares-xpansion/presolve/presolve.h"

#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <antares-xpansion/multisolver_interface/SolverConfig.h>
#include <antares-xpansion/multisolver_interface/SolverFactory.h>
#include <fmt/format.h>

#include "antares-xpansion/benders/benders_core/common.h"

const std::string PRESOLVE_CONTEXT{"Presolve"};

SolverAbstract::Ptr Presolve::init_solver(const PresolveOptions& options, Logger logger)
{
    SolverConfig config(options.SOLVER_NAME);

    if (!(config == "Xpress"))
    {
        std::cerr << "Invalid solver '" << config.Name()
                  << "'. Presolve is available only with Xpress." << std::endl;
        std::exit(1);
    }

    SolverFactory factory(logger);

    // TODO Shouldn't this flag be private in SolverFactory?
    if (!factory.isXpress_available_)
    {
        std::cerr << "Error: Xpress not available" << std::endl;
        std::exit(1);
    }

    SolverAbstract::Ptr solver_ptr = factory.create_solver(config);

    if (options.LOG_LEVEL > 0)
    {
        solver_ptr->set_output_log_level(options.LOG_LEVEL);
    }

    return solver_ptr;
}

std::unordered_map<int, int> Presolve::get_candidates_presolve_map(SolverAbstract* solver,
                                                                   std::vector<int>& candidatesId)
{
    // Get indices in reduced problem
    std::unordered_map<int, int> full2reduced;

    const int nbRows = solver->get_nrows();
    const int nbCols = solver->get_ncols();

    std::vector<int> rowmap(nbRows), colmap(nbCols);

    solver->get_presolve_map(rowmap.data(), colmap.data());

    // Since candidatesId is much smaller than colmap, sorting (mostly)
    // and searching the former is theoretically more efficient than
    // doing the opposite.
    std::sort(candidatesId.begin(), candidatesId.end());

    // Considering that candidates are added as the last columns of the
    // the problem, it can be more efficient to search backwards from colmap.
    // Note from Xpress API page: it is possible that the presolver will introduce
    // new rows or columns. For any added row or column the corresponding entry
    // returned will be -1
    for (int reduced_idx = colmap.size() - 1; reduced_idx >= 0; --reduced_idx)
    {
        const int full_idx = colmap[reduced_idx];
        if (std::binary_search(candidatesId.cbegin(), candidatesId.cend(), full_idx))
        {
            full2reduced.insert({full_idx, reduced_idx});

            if (full2reduced.size() == candidatesId.size())
            {
                // Found all candidates' indices
                break;
            }
        }
    }

    // TODO Should throw if full2reduced.size < candidatesId.size?
    return full2reduced;
}

void Presolve::reduce_problems(SolverAbstract::Ptr& solver,
                               const PresolveOptions& options,
                               Logger logger)
{
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    const auto structure_path = input_root_dir / options.STRUCTURE_FILE;
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / options.FULL_DIR;

    // Parse structure and get candidates' indices
    logger->display_message("Reading " + structure_path.string(),
                            LogUtils::LOGLEVEL::INFO,
                            PRESOLVE_CONTEXT);
    const CouplingMap full_couplings = CouplingMapGenerator::BuildInput(structure_path,
                                                                        logger.get(),
                                                                        PRESOLVE_CONTEXT);

    if (options.KEEP_FULL)
    {
        // Creates new folder to move full problems
        logger->display_message("Creating " + full_dir.string(),
                                LogUtils::LOGLEVEL::INFO,
                                PRESOLVE_CONTEXT);
        mkdir(full_dir);

        // Move full structure to 'full' folder
        std::filesystem::rename(structure_path, full_dir / options.STRUCTURE_FILE);
    }

    // ** Main part : creates coupling map for reduced problems **
    CouplingMap reduced_couplings = full_couplings;

    SolverIO solver_io;
    solver_io.configure(options.SOLVER_NAME, options.PROBLEMS_FORMAT);

    const size_t nb_prob_total{full_couplings.size() - 1};
    for (int nb_prob{0}; const auto& [filename, var_map]: full_couplings)
    {
        // TODO Can be done in parallel ?

        if (filename == options.MASTER_NAME) [[unlikely]]
        {
            // Keep master indices untouched
            // and only try to reduce the subproblems
            continue;
        }

        // Get all candidate indices in a sorted array
        std::vector<int> candidatesId(var_map.size());
        std::transform(var_map.cbegin(),
                       var_map.cend(),
                       candidatesId.begin(),
                       [](const auto pair) { return pair.second; });
        std::sort(candidatesId.begin(), candidatesId.end());

        // Read full problem
        const std::filesystem::path subproblem_path = input_root_dir / filename;

        solver_io.read(solver.get(), subproblem_path);

        // Keep the solver from removing candidate indices from subproblem
        solver->mark_indices_to_keep_presolve(0, candidatesId.size(), nullptr, candidatesId.data());

        solver->presolve_only();

        if (options.KEEP_FULL)
        {
            // Move full subproblem to 'full' folder
            std::filesystem::rename(subproblem_path, full_dir / filename);
        }

        // TODO Too verbose. Should only output if LOGLVL is set to DEBUG ?
        logger->display_message(
          fmt::format("Subproblem '{}' reduced: {} / {}.", filename, ++nb_prob, nb_prob_total),
          LogUtils::LOGLEVEL::DEBUG,
          PRESOLVE_CONTEXT);

        // Create a map [full_idx] -> reduced_idx for candidate indices
        const auto full2reduced = get_candidates_presolve_map(solver.get(), candidatesId);

        for (const auto& [var_name, idx]: var_map)
        {
            reduced_couplings.at(filename).at(var_name) = full2reduced.at(idx);
        }

        if (options.PROBLEMS_FORMAT == ProblemsFormat::SAVED_FILE)
        {
            /*
            TODO Won't be able to solve later with Benders
            When we force presolve_only, we interrupt the problem leaving it in a
            'presolved' state to be restored. Since .svf files are restored as they are,
            this 'presolved' state would pass to Benders.
            According to XPRESS API, it prevents any modification to the
            underlying matrix, blocking us to chg_obj in Benders for instance.

            One possible solution would be to export the problem as MPS,
            reload it and then export it again as .svf, but it's quite a hack
            It would work because the MPS format doesn't record the 'presolved' state

            A cleaner solution could be creating a new XPRSProb from the current
            matrix, i.e., XPRSprob(obj, mclind, mstart, dmatval, etc) and then
            export it, removing artificially the 'presolved' state.
            That would require some modifications to SolverXpress
            */
            SolverFactory factory(logger);
            solver = factory.copy_solver(solver);
            // logger->display_message("Export format won't allow benders to solve the problem",
            // LogUtils::LOGLEVEL::WARNING,
            // PRESOLVE_CONTEXT);
        }

        // TODO Why does it add a line break?
        // Write reduced problem MPS
        solver_io.write(solver.get(), subproblem_path);
    }

    // Write structure for reduced problem
    export_structure_file(structure_path, reduced_couplings);
    logger->display_message("Reduced " + options.STRUCTURE_FILE + " written",
                            LogUtils::LOGLEVEL::INFO,
                            PRESOLVE_CONTEXT);
}
