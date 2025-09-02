#include "antares-xpansion/presolve/presolve.h"

#include <algorithm>
#include <antares-xpansion/benders/benders_core/CouplingMapGenerator.h>
#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <antares-xpansion/multisolver_interface/SolverConfig.h>
#include <antares-xpansion/multisolver_interface/SolverFactory.h>
#include <fmt/format.h>
#include <stdexcept>

#include "antares-xpansion/benders/benders_core/common.h"

namespace
{

bool is_debug(LogUtils::LOGLEVEL lvl)
{
    return lvl == LogUtils::LOGLEVEL::DEBUG;
}

std::vector<int> build_sorted_candidate_indices(const VariableMap& var_map)
{
    std::vector<int> candidate_indices;
    candidate_indices.reserve(var_map.size());
    for (const auto& [_, idx]: var_map)
    {
        candidate_indices.push_back(idx);
    }
    std::sort(candidate_indices.begin(), candidate_indices.end());
    return candidate_indices;
}

void presolve_subproblem(SolverAbstract& solver,
                         SolverIO& solver_io,
                         const std::filesystem::path& subproblem_path,
                         std::span<const int> candidate_indices)
{
    solver_io.read(&solver, subproblem_path);
    solver.mark_indices_to_keep_presolve(0,
                                         static_cast<int>(candidate_indices.size()),
                                         nullptr,
                                         const_cast<int*>(candidate_indices.data()));
    solver.presolve_only();
}

void replace_structure_file(const std::filesystem::path& final_path,
                            const CouplingMap& reduced_couplings)
{
    // écriture vers fichier temporaire puis renommage atomique
    auto tmp = final_path;
    tmp += ".tmp";
    export_structure_file(tmp, reduced_couplings);
    std::error_code ec;
    std::filesystem::rename(tmp, final_path, ec);
    if (ec)
    {
        // tentative de nettoyage
        std::filesystem::remove(tmp);
        throw std::runtime_error(
          fmt::format("Failed to atomically replace '{}': {}", final_path.string(), ec.message()));
    }
}

void update_reduced_couplings_for_file(CouplingMap& reduced_couplings,
                                       const std::string& filename,
                                       const VariableMap& original_var_map,
                                       const std::unordered_map<int, int>& full2reduced)
{
    auto itMapFile = reduced_couplings.find(filename);
    if (itMapFile == reduced_couplings.end())
    {
        throw std::runtime_error("Filename missing in reduced_couplings after presolve");
    }
    auto& inner = itMapFile->second;
    for (const auto& [var_name, idx]: original_var_map)
    {
        auto itPos = full2reduced.find(idx);
        if (itPos == full2reduced.end())
        {
            throw std::runtime_error("Candidate index absent from mapping (logic error)");
        }
        inner[var_name] = itPos->second;
    }
}

} // namespace

SolverAbstract::Ptr Presolve::init_solver(const PresolveOptions& options,
                                          std::shared_ptr<ILogger>& logger)
{
    SolverConfig config(options.SOLVER_NAME);
    if (!(config == "Xpress"))
    {
        throw std::runtime_error(
          fmt::format("Invalid solver '{}' (only Xpress supported for presolve)", config.Name()));
    }

    SolverFactory factory(logger);
    if (!factory.is_xpress_available())
    {
        throw std::runtime_error("Xpress not available");
    }

    auto solver_ptr = factory.create_solver(config);
    if (options.LOG_LEVEL > 0)
    {
        solver_ptr->set_output_log_level(options.LOG_LEVEL);
    }
    return solver_ptr;
}

std::unordered_map<int, int> Presolve::get_candidates_presolve_map(
  const SolverAbstract& solver,
  std::span<const int> candidate_ids) const
{
    std::unordered_map<int, int> full2reduced;
    full2reduced.reserve(candidate_ids.size());

    const int nbRows = solver.get_nrows();
    const int nbCols = solver.get_ncols();
    if (nbRows < 0 || nbCols < 0)
    {
        throw std::runtime_error("Solver returned negative dimensions");
    }

    if (nbCols == 0 || candidate_ids.empty())
    {
        return full2reduced; // empty mapping acceptable
    }

    std::vector<int> rowmap(static_cast<size_t>(nbRows));
    std::vector<int> colmap(static_cast<size_t>(nbCols));
    const_cast<SolverAbstract&>(solver).get_presolve_map(rowmap.data(), colmap.data());

    if (!std::is_sorted(candidate_ids.begin(), candidate_ids.end()))
    {
        throw std::invalid_argument("candidate_ids must be sorted ascending");
    }

    // Considering that candidates are added as the last columns of the
    // the problem, it can be more efficient to search backwards from colmap.
    // Note from Xpress API page: it is possible that the presolver will introduce
    // new rows or columns. For any added row or column the corresponding entry
    // returned will be -1
    for (int reduced_idx = static_cast<int>(colmap.size()); reduced_idx-- > 0;)
    {
        const int full_idx = colmap[reduced_idx];
        if (full_idx < 0)
        {
            continue; // colonne ajoutée par presolve
        }
        if (std::binary_search(candidate_ids.begin(), candidate_ids.end(), full_idx))
        {
            full2reduced.emplace(full_idx, reduced_idx);
            if (full2reduced.size() == candidate_ids.size())
            {
                break;
            }
        }
    }

    if (full2reduced.size() != candidate_ids.size())
    {
        throw std::runtime_error("Presolve removed or failed to map some candidate columns");
    }
    return full2reduced;
}

void Presolve::safe_create_directory(const std::filesystem::path& dir)
{
    if (dir.empty())
    {
        return;
    }
    std::error_code ec;
    if (!std::filesystem::exists(dir) && !std::filesystem::create_directories(dir, ec))
    {
        throw std::runtime_error(
          fmt::format("Failed to create directory '{}': {}", dir.string(), ec.message()));
    }
}

void Presolve::safe_move_file(const std::filesystem::path& from, const std::filesystem::path& to)
{
    std::error_code ec;
    std::filesystem::create_directories(to.parent_path(), ec);
    std::filesystem::rename(from, to, ec);
    if (ec)
    {
        throw std::runtime_error(
          fmt::format("Failed to move '{}' -> '{}': {}", from.string(), to.string(), ec.message()));
    }
}

void Presolve::reduce_problems(std::shared_ptr<SolverAbstract> solver,
                               const PresolveOptions& options,
                               std::shared_ptr<ILogger> logger)
{
    const auto input_root_dir = std::filesystem::path(options.INPUTROOT);
    const auto structure_path = input_root_dir / options.STRUCTURE_FILE;
    const auto full_dir = std::filesystem::path(options.OUTPUTROOT) / options.FULL_DIR;

    logger->display_message("Reading " + structure_path.string(),
                            LogUtils::LOGLEVEL::INFO,
                            std::string(PRESOLVE_CONTEXT));
    const CouplingMap full_couplings = CouplingMapGenerator::BuildInput(structure_path,
                                                                        logger.get(),
                                                                        std::string(
                                                                          PRESOLVE_CONTEXT));

    if (options.KEEP_FULL)
    {
        logger->display_message("Creating " + full_dir.string(),
                                LogUtils::LOGLEVEL::INFO,
                                std::string(PRESOLVE_CONTEXT));
        safe_create_directory(full_dir);
        safe_move_file(structure_path, full_dir / options.STRUCTURE_FILE);
    }

    CouplingMap reduced_couplings = full_couplings;

    SolverIO solver_io;
    solver_io.configure(options.SOLVER_NAME, options.PROBLEMS_FORMAT);

    const size_t nb_prob_total{full_couplings.size() - 1};
    size_t processed = 0;

    for (const auto& [filename, var_map]: full_couplings)
    {
        if (filename == options.MASTER_NAME) [[unlikely]]
        {
            continue; // master intact
        }

        const auto candidate_indices = build_sorted_candidate_indices(var_map);
        const std::filesystem::path subproblem_path = input_root_dir / filename;

        presolve_subproblem(*solver, solver_io, subproblem_path, candidate_indices);

        if (options.KEEP_FULL)
        {
            safe_move_file(subproblem_path, full_dir / filename);
        }

        ++processed;
        if (is_debug(static_cast<LogUtils::LOGLEVEL>(options.LOG_LEVEL)))
        {
            logger->display_message(
              fmt::format("Subproblem '{}' reduced: {} / {}.", filename, processed, nb_prob_total),
              LogUtils::LOGLEVEL::DEBUG,
              std::string(PRESOLVE_CONTEXT));
        }

        const auto full2reduced = get_candidates_presolve_map(*solver, candidate_indices);
        update_reduced_couplings_for_file(reduced_couplings, filename, var_map, full2reduced);

        if (options.PROBLEMS_FORMAT == ProblemsFormat::OPTIMIZED)
        {
            /*
            When we force presolve_only, we interrupt the problem leaving it in a
            'presolved' state to be restored. Since .svf files are restored as they are,
            this 'presolved' state would pass to Benders.
            According to XPRESS API, it prevents any modification to the
            underlying matrix, blocking us to chg_obj in Benders for instance.

            A solution is to build a copy of the solver which is not in presolved state.
            */
            SolverFactory factory(logger);
            solver = factory.copy_solver(*solver); // neutraliser état presolved
        }

        solver_io.write(solver.get(), subproblem_path); // écrire problème réduit
    }

    // écriture atomique du fichier structure réduit
    replace_structure_file(structure_path, reduced_couplings);
    logger->display_message("Reduced " + options.STRUCTURE_FILE + " written",
                            LogUtils::LOGLEVEL::INFO,
                            std::string(PRESOLVE_CONTEXT));
}
