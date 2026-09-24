#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "BendersStructsDatas.h"
#include "CriterionComputation.h"

namespace Output
{
class OutputWriter;
struct SolutionData;
struct Iteration;
} // namespace Output

class BendersOuterLoopManager
{
public:
    using BendersSolutionFn = std::function<Output::SolutionData()>;
    using IterationFn = std::function<Output::Iteration(const WorkerMasterData&)>;

    BendersOuterLoopManager(CurrentIterationData& data,
                            const BendersRelevantIterationsData& relevant_iteration_data,
                            std::shared_ptr<Output::OutputWriter> writer,
                            BendersSolutionFn benders_solution_fn,
                            IterationFn iteration_fn);

    [[nodiscard]] CriteriaCurrentIterationData GetOuterLoopData() const;
    [[nodiscard]] std::vector<double> GetOuterLoopCriterionAtBestBenders() const;
    void UpdateOuterLoopSolution();
    [[nodiscard]] Output::SolutionData GetOuterLoopSolution() const;
    void SaveOuterLoopSolutionInOutputFile() const;
    void SaveCurrentOuterLoopIterationInOutputFile() const;
    void SetBilevelBestub(double bilevel_best_ub);

    void SetCriterionComputationInputs(
      const Benders::Criterion::CriterionInputData& criterion_input_data);
    Benders::Criterion::CriterionComputation& GetCriterionComputation();
    [[nodiscard]] const Benders::Criterion::CriterionComputation& GetCriterionComputation() const;

    void PushCriteriaForIteration(const std::vector<double>& criteria);
    void ClearCriteriaHistory();
    void UpdateMaxCriterionArea();

private:
    CurrentIterationData& data_;
    std::vector<std::vector<double>> criteria_vector_for_each_iteration_;
    const BendersRelevantIterationsData& relevant_iteration_data_;
    std::shared_ptr<Output::OutputWriter> writer_;
    BendersSolutionFn benders_solution_fn_;
    IterationFn iteration_fn_;
    Output::SolutionData outer_loop_solution_data_;
    Benders::Criterion::CriterionComputation criterion_computation_;
};
