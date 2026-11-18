#pragma once

#include <memory>
#include <vector>

#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/benders_core/BendersStructsDatas.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

// What the outer loop needs from the Benders algorithm, so that it does not depend on
// BendersBase itself.
class IOuterLoopFacade
{
public:
    virtual ~IOuterLoopFacade() = default;

    // Lifecycle
    virtual void Launch() = 0;
    virtual void Free() = 0;
    virtual void InitializeProblems() = 0;
    virtual void DoFreeProblems(bool val) = 0;
    virtual void InitData(double lambda, double lambda_min, double lambda_max) = 0;

    // State
    [[nodiscard]] virtual bool IsExceptionRaised() const = 0;
    virtual void IncrementRunNumber() = 0;
    [[nodiscard]] virtual int GetRunNumber() const = 0;
    [[nodiscard]] virtual CurrentIterationData GetCurrentIterationData() const = 0;
    [[nodiscard]] virtual WorkerMasterData BestIterationWorkerMaster() const = 0;
    [[nodiscard]] virtual BendersBaseOptions GetOptions() const = 0;
    virtual void UpdateOverallCosts() = 0;

    // Logging
    [[nodiscard]] virtual Logger GetLogger() const = 0;
    [[nodiscard]] virtual std::shared_ptr<MathLoggerDriver> GetMathLoggerDriver() const = 0;

    // Master operations
    [[nodiscard]] virtual std::vector<double> GetMasterObjectiveFunctionCoeffs() const = 0;
    virtual void SetMasterObjectiveFunctionCoeffsToZeros() = 0;
    virtual void SetMasterObjectiveFunction(const double* coeffs, int first, int last) = 0;
    [[nodiscard]] virtual const VariableMap& GetMasterVariableMap() const = 0;

    // Outer loop operations
    [[nodiscard]] virtual CriteriaCurrentIterationData GetOuterLoopData() const = 0;
    [[nodiscard]] virtual std::vector<double> GetOuterLoopCriterionAtBestBenders() const = 0;
    virtual void UpdateOuterLoopSolution() = 0;
    virtual void SaveCurrentOuterLoopIterationInOutputFile() const = 0;
    virtual void SetBilevelBestub(double val) = 0;
    virtual void SaveOuterLoopSolutionInOutputFile() const = 0;
};
