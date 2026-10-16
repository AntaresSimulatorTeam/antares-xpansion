#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/outer_loop/IOuterLoopFacade.h"

class OuterLoopFacade final: public IOuterLoopFacade
{
public:
    explicit OuterLoopFacade(pBendersBase benders);

    // Lifecycle
    void Launch() override;
    void Free() override;
    void InitializeProblems() override;
    void DoFreeProblems(bool val) override;
    void InitData(double lambda, double lambda_min, double lambda_max) override;

    // State
    [[nodiscard]] bool IsExceptionRaised() const override;
    void IncrementRunNumber() override;
    [[nodiscard]] int GetRunNumber() const override;
    [[nodiscard]] CurrentIterationData GetCurrentIterationData() const override;
    [[nodiscard]] WorkerMasterData BestIterationWorkerMaster() const override;
    [[nodiscard]] BendersBaseOptions GetOptions() const override;
    void UpdateOverallCosts() override;

    // Logging
    [[nodiscard]] Logger GetLogger() const override;
    [[nodiscard]] std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const override;

    // Master operations
    [[nodiscard]] std::vector<double> GetMasterObjectiveFunctionCoeffs() const override;
    void SetMasterObjectiveFunctionCoeffsToZeros() override;
    void SetMasterObjectiveFunction(const double* coeffs, int first, int last) override;
    [[nodiscard]] const VariableMap& GetMasterVariableMap() const override;

    // Outer loop operations
    [[nodiscard]] CriteriaCurrentIterationData GetOuterLoopData() const override;
    [[nodiscard]] std::vector<double> GetOuterLoopCriterionAtBestBenders() const override;
    void UpdateOuterLoopSolution() override;
    void SaveCurrentOuterLoopIterationInOutputFile() const override;
    void SetBilevelBestub(double val) override;
    void SaveOuterLoopSolutionInOutputFile() const override;

private:
    pBendersBase benders_;
};
