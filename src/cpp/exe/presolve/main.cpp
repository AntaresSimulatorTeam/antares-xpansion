#include <algorithm>
#include <filesystem>
#include <fmt/format.h>
#include <unordered_map>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/multisolver_interface/SolverXpress.h"

using XPRSPtr = std::shared_ptr<SolverXpress>;

XPRSPtr init_solver(BaseOptions& options, Logger logger)
{
    if (options.SOLVER_NAME != "Xpress")
    {
        // TODO if options.json is empty (since the orchestrator doesn't create it)
        // TODO create one using Xpress as the solver by default
        std::cerr << "Invalid solver '" << options.SOLVER_NAME
                  << "'. Will try to use Xpress instead." << std::endl;
        // std::exit(1);
        options.SOLVER_NAME = "Xpress";
    }

    SolverFactory factory(logger);

    // TODO Shouldn't this flag be private in SolverFactory?
    if (!factory.isXpress_available_)
    {
        std::cerr << "Error: Xpress not available" << std::endl;
        std::exit(1);
    }

    XPRSPtr solver_ptr = std::static_pointer_cast<SolverXpress>(
      factory.create_solver(options.SOLVER_NAME));

    // TODO Default is 0. Should keep it ?
    // solver.set_output_log_level(1);

    return solver_ptr;
}

std::unordered_map<int, int> get_presolve_map(SolverXpress& solver, const std::vector<int>& colind)
{
    // Get indices in reduced problem
    std::unordered_map<int, int> full2reduced;

    const int nbRows = solver.get_nrows();
    const int nbCols = solver.get_ncols();

    std::vector<int> rowmap(nbRows), colmap(nbCols);

    solver.get_presolve_map(rowmap.data(), colmap.data());

    for (int reduced_idx = 0; reduced_idx < colmap.size(); ++reduced_idx)
    {
        const int full_idx = colmap[reduced_idx];
        if (std::binary_search(colind.cbegin(), colind.cend(), full_idx))
        {
            full2reduced.insert({full_idx, reduced_idx});

            if (full2reduced.size() == colind.size())
            {
                // Found all candidates' indices
                break;
            }
        }
    }

    return full2reduced;
}

CouplingMap reduce_problems(SolverXpress& solver, const BaseOptions& options, Logger logger)
{
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    const auto structure_path = input_root_dir / options.STRUCTURE_FILE;
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / "full";

    // Parse structure and get candidates' indices
    logger->display_message("Reading " + structure_path.string());
    const CouplingMap full_couplings = CouplingMapGenerator::BuildInput(structure_path,
                                                                        logger.get(),
                                                                        "Presolve");

    // TODO Add option to keep or discard the full versions
    // TODO All filesystem operations can be done in a separated part I guess
    // Creates new folder to move full problems
    logger->display_message("Creating " + full_dir.string());
    mkdir(full_dir);

    // Move full structure to 'full' folder
    std::filesystem::rename(structure_path, full_dir / options.STRUCTURE_FILE);

    // ** Main part : creates coupling map for reduced problems **
    CouplingMap reduced_couplings = full_couplings;

    const size_t nb_prob_total{full_couplings.size() - 1};
    for (int nb_prob{0}; const auto& [filename, var_map]: full_couplings)
    {
        if (filename == options.MASTER_NAME) [[unlikely]]
        {
            // Keep master indices untouched
            // and only try to reduce the subproblems
            continue;
        }

        // Get all candidate indices in a sorted array
        std::vector<int> colind(var_map.size());
        std::transform(var_map.cbegin(),
                       var_map.cend(),
                       colind.begin(),
                       [](const auto pair) { return pair.second; });
        std::sort(colind.begin(), colind.end());

        // Read full problem
        const std::filesystem::path subproblem_path = input_root_dir / filename;

        // TODO See about the keeprows thing
        solver.read_prob_mps(subproblem_path);

        // Keep the solver from removing candidate indices from subproblem
        solver.mark_indices_to_keep_presolve(0, colind.size(), nullptr, colind.data());

        solver.presolve_only();

        // Move full subproblem to 'full' folder
        // TODO Add option to keep or discard the full versions
        // TODO All filesystem operations can be done in a separated part I guess
        std::filesystem::rename(subproblem_path, full_dir / filename);

        logger->display_message(
          fmt::format("Subproblem '{}' reduced: {} / {}", filename, ++nb_prob, nb_prob_total));

        // Write reduced problem MPS
        solver.write_prob_mps(subproblem_path);

        // Create a map [full_idx] -> reduced_idx for candidate indices
        const auto full2reduced = get_presolve_map(solver, colind);

        for (const auto& [var_name, idx]: var_map)
        {
            reduced_couplings.at(filename).at(var_name) = full2reduced.at(idx);
        }
    }

    return reduced_couplings;
}

void write_reduced_problems(const CouplingMap& reduced_couplings,
                            const BaseOptions& options,
                            Logger logger)
{
    const auto structure_path{std::filesystem::path(options.INPUTROOT) / options.STRUCTURE_FILE};

    // Write structure for reduced problem
    export_structure_file(structure_path, reduced_couplings);
    logger->display_message("Reduced " + options.STRUCTURE_FILE + " written");
}

int main(int argc, char** argv)
{
    usage(argc);
    BaseOptions options{SimulationOptions(argv[1]).get_base_options()};
    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("Starting presolve");

    XPRSPtr solver_ptr = init_solver(options, logger);

    const CouplingMap reduced_couplings = reduce_problems(*solver_ptr, options, logger);

    write_reduced_problems(reduced_couplings, options, logger);

    logger->display_message("Presolve finished");

    return 0;
}
