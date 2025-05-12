#include <algorithm>
#include <filesystem>
#include <fmt/format.h>
#include <unordered_map>

#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/multisolver_interface/environment.h"

void zero_status_check(int status,
                       const std::string& failed_action,
                       const std::string& log_location)
{
    if (status != 0)
    {
        throw LogUtils::XpansionError<std::runtime_error>(
          fmt::format("Failed to {}: invalid status {} (expected 0).", failed_action, status),
          log_location);
    }
}

void XPRS_CC xpressMessageCb(XPRSprob my_prob, void* object, const char* msg, int len, int msgtype)
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

XPRSprob init_xpress(BaseOptions& options, Logger logger)
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

    LoadXpress::XpressLoader xpressLoader(logger);
    if (!xpressLoader.XpressIsCorrectlyInstalled(true))
    {
        std::cerr << "Error: Xpress not available" << std::endl;
        std::exit(1);
    }

    xpressLoader.initXpressEnv();
    int status = LoadXpress::XPRSinit(NULL);
    zero_status_check(status, "initialize XPRESS environment", LOGLOCATION);

    XPRSprob xprsProb;

    status = LoadXpress::XPRScreateprob(&xprsProb);
    zero_status_check(status, "create XPRESS problem", LOGLOCATION);

    // TODO Probably should use 'XPRSsetcbmessage' as in SolverXpress.cpp
    status = LoadXpress::XPRSaddcbmessage(xprsProb, xpressMessageCb, NULL, 0);
    zero_status_check(status, "add message callback to solver", LOGLOCATION);

    status = LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
    zero_status_check(status, "set log level", LOGLOCATION);

    return xprsProb;
}

CouplingMap get_full_coupling(const BaseOptions& options, Logger logger)
{
    // Parse structure and get candidates' indices
    const auto structure_path{std::filesystem::path(options.INPUTROOT) / options.STRUCTURE_FILE};

    const CouplingMap full_coupling = CouplingMapGenerator::BuildInput(structure_path,
                                                                       logger.get(),
                                                                       "Presolve");

    logger->display_message(structure_path.string() + " read");

    return full_coupling;
}

std::unordered_map<int, int> get_presolve_map(XPRSprob xprsProb, const std::vector<int>& indices)
{
    // Get indices in reduced problem
    std::unordered_map<int, int> full2reduced;

    int nbCols(0), nbRows(0);

    int status = LoadXpress::XPRSgetintattrib(xprsProb, XPRS_COLS, &nbCols);
    zero_status_check(status, "get number of columns", LOGLOCATION);

    status = LoadXpress::XPRSgetintattrib(xprsProb, XPRS_ROWS, &nbRows);
    zero_status_check(status, "get number of rows", LOGLOCATION);

    std::vector<int> col_map(nbCols), row_map(nbRows);

    status = LoadXpress::XPRSgetpresolvemap(xprsProb, row_map.data(), col_map.data());
    zero_status_check(status, "get presolve map", LOGLOCATION);

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

    return full2reduced;
}

CouplingMap get_reduced_coupling(XPRSprob xprsProb,
                                 const CouplingMap& full_couplings,
                                 const BaseOptions& options,
                                 Logger logger)
{
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / "full";

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
        std::vector<int> indices(var_map.size());
        std::transform(var_map.cbegin(),
                       var_map.cend(),
                       indices.begin(),
                       [](const auto pair) { return pair.second; });
        std::sort(indices.begin(), indices.end());

        // Read full problem
        const std::filesystem::path subproblem_path = input_root_dir / filename;

        int status = LoadXpress::XPRSreadprob(xprsProb, subproblem_path.c_str(), "");
        zero_status_check(status, "read subproblem " + filename, LOGLOCATION);

        // Keep the solver from removing these indices from subproblem
        status = LoadXpress::XPRSloadsecurevecs(xprsProb,
                                                0,
                                                indices.size(),
                                                nullptr,
                                                indices.data());
        zero_status_check(status, "fix variables from subproblem", LOGLOCATION);

        // Presolve only: Set max iteration to 0
        status = LoadXpress::XPRSsetintcontrol(xprsProb, XPRS_LPITERLIMIT, 0);
        zero_status_check(status, "set solver to only run presolve", LOGLOCATION);

        // Run solver
        status = LoadXpress::XPRSlpoptimize(xprsProb, "");
        zero_status_check(status, "run presolve", LOGLOCATION);

        // Move full subproblem to 'full' folder
        // TODO Add option to keep or discard the full versions
        // TODO All filesystem operations can be done in a separated part I guess
        std::filesystem::rename(subproblem_path, full_dir / filename);

        logger->display_message(
          fmt::format("Subproblem '{}' reduced: {} / {}", filename, ++nb_prob, nb_prob_total));

        // Write reduced problem MPS
        status = LoadXpress::XPRSwriteprob(xprsProb, subproblem_path.c_str(), "");
        zero_status_check(status, "write subproblem " + filename, LOGLOCATION);

        // Create a map [full_idx] -> reduced_idx for candidate indices
        std::unordered_map<int, int> full2reduced = get_presolve_map(xprsProb, indices);

        for (const auto& [var_name, idx]: var_map)
        {
            reduced_couplings.at(filename).at(var_name) = full2reduced.at(idx);
        }
    }

    return reduced_couplings;
}

void create_full_problems_dir(const BaseOptions& options, Logger logger)
{
    const auto structure_path{std::filesystem::path(options.INPUTROOT) / options.STRUCTURE_FILE};

    // Creates new folder to move full problems
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / "full";
    mkdir(full_dir);

    // Move full structure to 'full' folder
    std::filesystem::rename(structure_path, full_dir / options.STRUCTURE_FILE);
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

void free_xpress(XPRSprob xprsProb, const BaseOptions& options, Logger logger)
{
    const int status = LoadXpress::XPRSdestroyprob(xprsProb);
    xprsProb = nullptr;

    if (status)
    {
        std::cerr << "Failed to destroy XPRESS problem with status: " << status << " "
                  << LOGLOCATION << std::endl;
    }
}

int main(int argc, char** argv)
{
    usage(argc);
    BaseOptions options{SimulationOptions(argv[1]).get_base_options()};
    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("Starting presolve");

    XPRSprob xprsProb = init_xpress(options, logger);

    const CouplingMap full_coupling = get_full_coupling(options, logger);

    create_full_problems_dir(options, logger);

    const CouplingMap reduced_coupling = get_reduced_coupling(xprsProb,
                                                              full_coupling,
                                                              options,
                                                              logger);

    write_reduced_problems(reduced_coupling, options, logger);

    free_xpress(xprsProb, options, logger);

    logger->display_message("Presolve finished");

    return 0;
}
