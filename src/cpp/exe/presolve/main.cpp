#include <algorithm>
#include <filesystem>
#include <fmt/format.h>
#include <unordered_map>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/multisolver_interface/environment.h"

void XPRS_CC Message(XPRSprob my_prob, void* object, const char* msg, int len, int msgtype)
{
    switch (msgtype)
    {
    case 4:
        [[fallthrough]]; /* error */
    case 3:
        [[fallthrough]]; /* warning */
    case 2:
        [[fallthrough]]; /* not used */
    case 1:              /* information */
        printf("%s\n", msg);
        break;
    default: /* exiting - buffers need flushing */
        fflush(stdout);
        break;
    }
}

int main(int argc, char** argv)
{
    usage(argc);
    BaseOptions options{SimulationOptions(argv[1]).get_base_options()};
    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("Starting presolve");

    if (options.SOLVER_NAME != "Xpress")
    {
        // TODO if options.json is empty (since the orchestrator doesn't create it)
        // TODO create one using Xpress as the solver by default
        std::cerr << "Invalid solver '" << options.SOLVER_NAME
                  << "'. Will try to use Xpress instead." << std::endl;
        // std::exit(1);
        options.SOLVER_NAME = "Xpress";
    }

    // Initialize Xpress
    LoadXpress::XpressLoader xpressLoader(logger);

    if (!xpressLoader.XpressIsCorrectlyInstalled(true))
    {
        std::cerr << "Error: Xpress not available" << std::endl;
        std::exit(1);
    }
    xpressLoader.initXpressEnv();

    // Create Problem
    XPRSprob xprsProb;
    LoadXpress::XPRScreateprob(&xprsProb);

    LoadXpress::XPRSinit(NULL);
    LoadXpress::XPRSaddcbmessage(xprsProb, Message, NULL, 0); // TODO Check this callback
    LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);

    // Parse structure and get candidates' indices
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    const auto structure_path(input_root_dir / options.STRUCTURE_FILE);

    const CouplingMap full_couplings = CouplingMapGenerator::BuildInput(structure_path,
                                                                        logger.get(),
                                                                        "Presolve");

    logger->display_message(structure_path.string() + " read");

    // TODO Move this part into its own function
    // Creates new folder to move full problems
    const std::string full_prefix{"full"};
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / full_prefix;
    if (!std::filesystem::exists(full_dir) && !std::filesystem::create_directories(full_dir))
    {
        std::cerr << "Could not create " << full_dir << "folder" << std::endl;
    }

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

        std::vector<int> indices(var_map.size());
        std::transform(var_map.cbegin(),
                       var_map.cend(),
                       indices.begin(),
                       [](const auto pair) { return pair.second; });

        // Read full problem MPS
        const std::filesystem::path subproblem_path = input_root_dir / filename;
        LoadXpress::XPRSreadprob(xprsProb, subproblem_path.c_str(), "");

        // Keep the solver from removing these indices from subproblem
        LoadXpress::XPRSloadsecurevecs(xprsProb, 0, indices.size(), nullptr, indices.data());

        // Presolve only: Set max iteration to 0
        LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_LPITERLIMIT, 0);

        // Run solver
        LoadXpress::XPRSlpoptimize(xprsProb, "");

        // Move full subproblem to 'full' folder
        // TODO Add option to keep or discard the full versions
        // TODO All filesystem operations can be done in a separated part I guess
        std::filesystem::rename(subproblem_path, full_dir / filename);

        logger->display_message(
          fmt::format("Subproblem '{}' reduced: {} / {}", filename, ++nb_prob, nb_prob_total));

        // Write reduced problem MPS
        LoadXpress::XPRSwriteprob(xprsProb, subproblem_path.c_str(), "");

        // Get indices in reduced problem
        int nbCols(0);
        int nbRows(0);
        LoadXpress::XPRSgetintattrib(xprsProb, XPRS_COLS, &nbCols);
        LoadXpress::XPRSgetintattrib(xprsProb, XPRS_ROWS, &nbRows);

        std::vector<int> col_map(nbCols);
        std::vector<int> row_map(nbRows);
        LoadXpress::XPRSgetpresolvemap(xprsProb, row_map.data(), col_map.data());

        // Create a map [full_idx] -> reduced_idx for candidate indices
        std::unordered_map<int, int> full2reduced;
        std::sort(indices.begin(), indices.end());

        for (int reduced_idx = 0; reduced_idx < col_map.size(); ++reduced_idx)
        {
            const int full_idx = col_map[reduced_idx];
            if (std::binary_search(indices.cbegin(), indices.cend(), full_idx))
            {
                full2reduced.insert({full_idx, reduced_idx});

                if (full2reduced.size() == indices.size())
                {
                    // Found all candidates' indices
                    break;
                }
            }
        }

        for (const auto& [var_name, idx]: var_map)
        {
            reduced_couplings.at(filename).at(var_name) = full2reduced.at(idx);
        }
    }

    // Write structure for reduced problem
    export_structure_file(structure_path, reduced_couplings);
    logger->display_message("Reduced " + options.STRUCTURE_FILE + " written");

    logger->display_message("Presolve finished");

    return 0;
}
