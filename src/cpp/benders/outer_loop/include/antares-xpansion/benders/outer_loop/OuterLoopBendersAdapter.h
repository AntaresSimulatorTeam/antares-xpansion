#pragma once

#include <memory>
#include <vector>

#include "antares-xpansion/benders/benders_core/BendersBase.h"

namespace Outerloop
{

/**
 * @brief Adapter that encapsulates outer-loop-specific operations on the Benders engine.
 *
 * Owns the outer-loop state (lambda context, benders run number, bilevel best ub,
 * outer-loop solution) used by the outer-loop orchestration, and delegates engine
 * lifecycle and master-problem operations to BendersBase. The engine only exposes
 * generic accessors; it no longer holds outer-loop state.
 */
class OuterLoopBendersAdapter
{
public:
    explicit OuterLoopBendersAdapter(pBendersBase benders);

    // -- Outer-loop state --------------------------------------------------
    [[nodiscard]] CriteriaCurrentIterationData GetOuterLoopData() const;
    [[nodiscard]] std::vector<double> GetOuterLoopCriterionAtBestBenders() const;

    /// Set the lambda context for the next benders run. The engine is not touched.
    void InitOuterLoopData(double lambda, double lambda_min, double lambda_max);

    void SetBilevelBestub(double bilevel_best_ub);
    void UpdateOuterLoopSolution();
    [[nodiscard]] Output::SolutionData GetOuterLoopSolution() const;

    int GetBendersRunNumber() const;
    void IncrementBendersRunNumber();

    // -- Output file (owned by the outer loop while it is active) ----------
    void SaveOuterLoopSolutionInOutputFile() const;
    void SaveCurrentOuterLoopIterationInOutputFile();

    // -- Engine lifecycle ----------------------------------------------------
    void DoFreeProblems(bool free_problems);
    void InitializeProblems();

    /**
     * @brief Run a benders execution and refresh the outer-loop state.
     *
     * Seeds the engine run context (benders run number, lambda values, bilevel
     * best ub) so per-iteration math logging reports the current outer-loop
     * values, runs the benders method, then refreshes the adapter-owned state.
     */
    void Launch();
    void Free();
    [[nodiscard]] bool IsExceptionRaised() const;

    // -- Engine views used by the outer-loop computations -------------------
    [[nodiscard]] Logger GetLogger() const;
    [[nodiscard]] std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const;
    void UpdateOverallCosts();

    [[nodiscard]] std::vector<double> MasterObjectiveFunctionCoeffs() const;
    void SetMasterObjectiveFunctionCoeffsToZeros() const;
    void SetMasterObjectiveFunction(const double* coeffs, int first, int last) const;
    [[nodiscard]] const VariableMap& MasterVariables() const;

    [[nodiscard]] WorkerMasterData BestIterationWorkerMaster() const;
    [[nodiscard]] CurrentIterationData GetCurrentIterationData() const;
    [[nodiscard]] bool DoOuterLoop() const;

private:
    void RefreshOuterLoopStateFromBenders();

    pBendersBase benders_;
    // Outer-loop fields (lambda*, benders_num_run, outer_loop_bilevel_best_ub)
    // are adapter-owned; criteria fields are refreshed from the engine after
    // each benders run.
    CriteriaCurrentIterationData current_outer_loop_data_;
    std::vector<double> outer_loop_criterion_at_best_benders_;
    Output::SolutionData outer_loop_solution_data_;
};

} // namespace Outerloop