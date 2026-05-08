#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"

namespace Outerloop
{

struct LambdaParameters
{
    double lambda = 0.0;
    double lambda_min = 0.0;
    double lambda_max = 0.0;
};

/**
 * @brief Adapter that encapsulates outer-loop-specific operations on BendersBase.
 *
 * Owns the outer-loop state used by orchestration and delegates lifecycle and
 * master-problem operations to BendersBase.
 */
class OuterLoopBendersAdapter
{
public:
    explicit OuterLoopBendersAdapter(pBendersBase benders);

    CriteriaCurrentIterationData GetOuterLoopData() const;
    std::vector<double> GetOuterLoopCriterionAtBestBenders() const;

    void InitOuterLoopData(double lambda, double lambda_min, double lambda_max);

    void SaveOuterLoopSolutionInOutputFile() const;
    void SaveCurrentOuterLoopIterationInOutputFile();

    void SetBilevelBestub(double bilevel_best_ub);
    void UpdateOuterLoopSolution();
    Output::SolutionData GetOuterLoopSolution() const;

    void UpdateOverallCosts();

    int GetBendersRunNumber() const;
    void IncrementBendersRunNumber();

    [[nodiscard]] double GetBilevelBestub() const;
    [[nodiscard]] LambdaParameters GetLambdaParameters() const;

    [[nodiscard]] Logger GetLogger() const;
    [[nodiscard]] std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const;
    void DoFreeProblems(bool free_problems);
    void InitializeProblems();

    /**
     * @brief Run BendersBase::launch() and refresh adapter-side outer-loop state.
     *
     * Why: callers of the outer loop must observe a fresh outer-loop snapshot after
     * every Benders run. Folding the refresh into Launch() removes the implicit
     * "remember to refresh" obligation.
     */
    void Launch();

    void Free();
    [[nodiscard]] bool IsExceptionRaised() const;

    [[nodiscard]] std::vector<double> MasterObjectiveFunctionCoeffs() const;
    void SetMasterObjectiveFunctionCoeffsToZeros() const;
    void SetMasterObjectiveFunction(const double* coeffs, int first, int last) const;
    [[nodiscard]] const VariableMap& MasterVariables() const;

    [[nodiscard]] WorkerMasterData BestIterationWorkerMaster() const;
    [[nodiscard]] CurrentIterationData GetCurrentIterationData() const;
    [[nodiscard]] bool DoOuterLoop() const;

private:
    void RefreshOuterLoopStateFromBenders();
    void UpdateCurrentOuterLoopIterationSnapshot();

    pBendersBase benders_;

    // Single source of truth for adapter-owned outer-loop fields (lambda*, benders_num_run,
    // outer_loop_bilevel_best_ub) plus a snapshot of Benders-owned criteria fields refreshed
    // by Launch().
    CriteriaCurrentIterationData current_outer_loop_data_;
    std::vector<double> outer_loop_criterion_at_best_benders_;
    Output::SolutionData outer_loop_solution_data_;

    std::optional<Output::Iteration> current_outer_loop_iteration_;
    int current_outer_loop_iteration_num_ = 0;
};

} // namespace Outerloop
