#include <algorithm>
#include <filesystem>
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
        std::cerr << "Error: Invalid solver used. Only Xpress is accepted " << std::endl;
        std::exit(1);
    }

    if (!LoadXpress::XpressLoader(logger).XpressIsCorrectlyInstalled(true))
    {
        std::cerr << "Error: Xpress not available" << std::endl;
        std::exit(1);
    }

    // Initialize Xpress
    LoadXpress::XpressLoader xpressLoader;
    xpressLoader.initXpressEnv();
    XPRSprob xprsProb;
    LoadXpress::XPRSinit(NULL);
    LoadXpress::XPRSaddcbmessage(xprsProb, Message, NULL, 0);
    LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);

    // Create Problem
    LoadXpress::XPRScreateprob(&xprsProb);

    // Parse structure and get candidates' indices
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    auto structure_path(input_root_dir / options.STRUCTURE_FILE);

    const CouplingMap full_couplings = CouplingMapGenerator::BuildInput(structure_path,
                                                                        logger.get(),
                                                                        "Presolve");

    // Rename STRUCTURE_FILE to STRUCTURE_FILE_full,
    const auto ext = structure_path.extension();
    structure_path.replace_filename(structure_path.stem().string() + "_full")
      .replace_extension(ext);

    logger->display_message(structure_path.string() + " created");

    CouplingMap reduced_couplings = full_couplings;

    const std::string reduced_prefix = "reduced-";
    for (const auto& [filename, var_map]: full_couplings)
    {
        if (filename == "master")
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
        const std::filesystem::path full_mps_path = input_root_dir / filename;
        LoadXpress::XPRSreadprob(xprsProb, full_mps_path.c_str(), "");

        // Keep the solver from removing these indices from subproblem
        LoadXpress::XPRSloadsecurevecs(xprsProb, 0, indices.size(), nullptr, indices.data());

        // Presolve only: Set max iteration to 0
        LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_LPITERLIMIT, 0);

        // Run solver
        LoadXpress::XPRSlpoptimize(xprsProb, "");

        // Write reduced problem MPS
        const std::filesystem::path reduced_mps_path = input_root_dir / (reduced_prefix + filename);
        LoadXpress::XPRSwriteprob(xprsProb, reduced_mps_path.c_str(), "");

        logger->display_message(reduced_mps_path.string() + " written");

        // Get indices in reduced problem
        int nbCols(0);
        int nbRows(0);
        LoadXpress::XPRSgetintattrib(xprsProb, XPRS_COLS, &nbCols);
        LoadXpress::XPRSgetintattrib(xprsProb, XPRS_ROWS, &nbRows);

        std::vector<int> col_map(nbCols);
        std::vector<int> row_map(nbRows);
        LoadXpress::XPRSgetpresolvemap(xprsProb, row_map.data(), col_map.data());

        // Create a map [full_idx] -> reduced_idx
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
                    // Found all indices
                    break;
                }
            }
        }

        for (const auto& [var_name, idx]: var_map)
        {
            reduced_couplings[filename][var_name] = full2reduced[idx];
        }
    }

    logger->display_message("Presolve finished");

    // Write structure for reduced problem
    export_structure_file(input_root_dir / options.STRUCTURE_FILE, reduced_couplings);
    logger->display_message("Reduced " + options.STRUCTURE_FILE + "written");

    return 0;
}
