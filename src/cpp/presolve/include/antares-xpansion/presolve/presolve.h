#pragma once
#include <antares-xpansion/multisolver_interface/SolverAbstract.h>
#include <antares-xpansion/xpansion_interfaces/ILogger.h>
#include <filesystem>
#include <span>
#include <unordered_map>

struct PresolveOptions;

class Presolve
{
public:
    // Initialise un solver; lève std::runtime_error si indisponible
    SolverAbstract::Ptr init_solver(const PresolveOptions& options, const Logger& logger);

    // Construit le mapping entre indices candidats (full) et indices réduits après presolve.
    // Pré-condition: solver déjà en état presolve_only.
    // Post-condition: mapping size == candidate_ids.size() sinon exception.
    [[nodiscard]] std::unordered_map<int, int> get_candidates_presolve_map(
      const SolverAbstract& solver,
      std::span<const int> candidate_ids) const;

    void reduce_problems(SolverAbstract::Ptr& solver,
                         const PresolveOptions& options,
                         const Logger& logger);

private:
    static void safe_create_directory(const std::filesystem::path& dir);
    static void safe_move_file(const std::filesystem::path& from, const std::filesystem::path& to);
};
