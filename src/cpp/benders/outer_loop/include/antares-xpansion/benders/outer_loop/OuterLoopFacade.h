#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"

class OuterLoopFacade
{
public:
    explicit OuterLoopFacade(pBendersBase benders);

    // Lifecycle
    void Launch();
    void Free();
    void InitializeProblems();
    void DoFreeProblems(bool val);
    void InitData(double lambda, double lambda_min, double lambda_max);

    // State
    [[nodiscard]] bool IsExceptionRaised() const;
    void IncrementRunNumber();
    [[nodiscard]] int GetRunNumber() const;
    [[nodiscard]] CurrentIterationData GetCurrentIterationData() const;
    [[nodiscard]] WorkerMasterData BestIterationWorkerMaster() const;
    [[nodiscard]] BendersBaseOptions GetOptions() const;
    void UpdateOverallCosts();

    // Logging
    [[nodiscard]] Logger GetLogger() const;
    [[nodiscard]] std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const;

    // Master operations
    [[nodiscard]] std::vector<double> GetMasterObjectiveFunctionCoeffs() const;
    void SetMasterObjectiveFunctionCoeffsToZeros();
    void SetMasterObjectiveFunction(const double* coeffs, int first, int last);
    [[nodiscard]] const VariableMap& GetMasterVariableMap() const;

    // Outer loop operations
    [[nodiscard]] CriteriaCurrentIterationData GetOuterLoopData() const;
    [[nodiscard]] std::vector<double> GetOuterLoopCriterionAtBestBenders() const;
    void UpdateOuterLoopSolution();
    void SaveCurrentOuterLoopIterationInOutputFile() const;
    void SetBilevelBestub(double val);
    void SaveOuterLoopSolutionInOutputFile() const;

private:
    pBendersBase benders_;
    std::shared_ptr<BendersMasterManager> master_manager_;
    std::shared_ptr<BendersOuterLoopManager> outer_loop_manager_;
};
