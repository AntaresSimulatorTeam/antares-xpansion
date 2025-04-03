#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/merge_master_mps/StandardLp.h"
#include "antares-xpansion/benders/merge_master_mps/MasterCoupling.h"

#include <filesystem>
#include <utility>

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

/**
 * Limitation: on windows may not support master problem with full path as name
 */
void MergeMasterMPS::launch()
{
    _logger->display_message("Inside MergeMasterMPS::launch()");
    const auto inputRootDir = std::filesystem::path(_options.INPUTROOT);
    auto structure_path(inputRootDir / _options.STRUCTURE_FILE);
    auto [general_data, coupling_map] = MasterCouplingMapGenerator::BuildInput(structure_path, _logger.get());

    SolverFactory factory;
    SolverAbstract::Ptr mergedSolver_l = factory.create_solver(_options.SOLVER_TO_USE);
    mergedSolver_l->set_output_log_level(_options.LOG_LEVEL);

    CouplingMap x_mps_id;
    // int cntProblems_l(0);

    _logger->display_message("Merging master problems...");
    for (const auto& [node_name, node_data] : coupling_map)
    {
    //     auto problem_name(inputRootDir / (kvp.first));
    //     SolverAbstract::Ptr solver_l = factory.create_solver(_options.SOLVER_TO_USE);
    //     solver_l->set_output_log_level(_options.LOG_LEVEL);

    //     // Read the problem
    //     solver_l->read_prob_mps(problem_name);

    //     StandardLp lpData(*solver_l);
    //     std::string varPrefix_l = "prob" + std::to_string(cntProblems_l) + "_";

    //     lpData.append_in(mergedSolver_l, varPrefix_l);

    //     for (const auto& x: kvp.second)
    //     {
    //         int col_index = mergedSolver_l->get_col_index(varPrefix_l + x.first);
    //         if (col_index == -1)
    //         {
    //             std::cerr << LOGLOCATION << "missing variable " << x.first << " in " << kvp.first
    //                       << " supposedly renamed to " << varPrefix_l + x.first << ".";
    //             mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT)
    //                                           / "mergeError.lp");
    //             mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
    //                                            / ("mergeError" + MPS_SUFFIX));
    //             std::exit(1);
    //         }
    //         else
    //         {
    //             x_mps_id[x.first][kvp.first] = col_index;
    //         }
    //     }

    //     ++cntProblems_l;
    }

    // IntVector mstart;
    // IntVector cindex;
    // DblVector values;
    // int nrows(0);
    // int neles(0);
    // size_t neles_reserve(0);
    // size_t nrows_reserve(0);
    // for (const auto& kvp: x_mps_id)
    // {
    //     neles_reserve += kvp.second.size() * (kvp.second.size() - 1);
    //     nrows_reserve += kvp.second.size() * (kvp.second.size() - 1) / 2;
    // }
    // _logger->display_message("About to add " + std::to_string(nrows_reserve)
    //                          + " coupling constraints");
    // values.reserve(neles_reserve);
    // cindex.reserve(neles_reserve);
    // mstart.reserve(nrows_reserve + 1);

    // // adding coupling constraints
    // for (const auto& kvp: x_mps_id)
    // {
    //     const std::string var_name(kvp.first);
    //     _logger->display_message(var_name);
    //     bool is_first(true);
    //     int id1(-1);
    //     std::string first_mps;
    //     for (const auto& mps: kvp.second)
    //     {
    //         if (is_first)
    //         {
    //             is_first = false;
    //             first_mps = mps.first;
    //             id1 = mps.second;
    //         }
    //         else
    //         {
    //             int id2 = mps.second;
    //             mstart.push_back(neles);
    //             cindex.push_back(id1);
    //             values.push_back(1);
    //             ++neles;

    //             cindex.push_back(id2);
    //             values.push_back(-1);
    //             ++neles;
    //             ++nrows;
    //         }
    //     }
    // }
    // mstart.push_back(neles);

    // DblVector rhs(nrows, 0);
    // CharVector sense(nrows, 'E');
    // solver_addrows(*mergedSolver_l, sense, rhs, {}, mstart, cindex, values);

    // _logger->display_message("Problems merged.");
    // _logger->display_message("Writing mps file");
    // mergedSolver_l->write_prob_mps(std::filesystem::path(_options.OUTPUTROOT)
    //                                / ("log_merged" + MPS_SUFFIX));
    // _logger->display_message("Writing lp file");
    // mergedSolver_l->write_prob_lp(std::filesystem::path(_options.OUTPUTROOT) / "log_merged.lp");

}
