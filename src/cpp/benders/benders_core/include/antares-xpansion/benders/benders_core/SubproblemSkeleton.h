#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "SkeletonCoefficientSet.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

/**
 * The per-subproblem coefficient data read from the skeleton CSVs, and the
 * knowledge of how to stamp it onto a solver.
 *
 * In skeleton mode a single solver instance is loaded once from sub/sub.mps and
 * reused for every subproblem; the three sets below hold what differs between
 * them (matrix coefficients, objective, right-hand side). They are always
 * loaded together and always applied together, which is why they live here
 * rather than as loose members of SubproblemWorkerFactory.
 */
class SubproblemSkeleton
{
public:
    /// Reads the six CSVs under `sub_dir`, keeping only the rows whose key is in
    /// `sub_problem_names`. `solver` is used to resolve row/column names to indices
    /// and to capture the skeleton's own objective, which ApplyTo() re-weights.
    void Load(const std::filesystem::path& sub_dir,
              std::vector<std::string> sub_problem_names,
              const std::shared_ptr<SolverAbstract>& solver,
              Logger& logger,
              AbortFunc abort_func = nullptr);

    /// Stamps `sub_name`'s coefficients, objective and right-hand side onto `solver`,
    /// the objective being scaled by `slave_weight`.
    void ApplyTo(SolverAbstract& solver, const std::string& sub_name, double slave_weight);

    /// Number of subproblems this skeleton holds right-hand sides for. Only the
    /// names passed to Load() are counted, so under MPI this is a rank-local count.
    int SubproblemCountForThisRank() const;

private:
    SkeletonCoefficientSet coef_set_;
    SkeletonCoefficientSet obj_set_;
    SkeletonCoefficientSet rhs_set_;

    /// The objective of sub.mps as loaded, before any subproblem was stamped onto
    /// the solver. Kept so each ApplyTo() re-weights from the skeleton values
    /// instead of from the previous subproblem's already-weighted ones.
    std::vector<double> skeleton_obj_coeffs_;
    std::vector<int> all_col_indices_;
};
