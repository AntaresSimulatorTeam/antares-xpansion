#include "antares-xpansion/benders/benders_core/BendersOuterLoopManager.h"

#include "antares-xpansion/benders/output/OutputWriter.h"

BendersOuterLoopManager::BendersOuterLoopManager(
  CurrentIterationData& data,
  const std::vector<std::vector<double>>& criteria_vector_for_each_iteration,
  const BendersRelevantIterationsData& relevant_iteration_data,
  std::shared_ptr<Output::OutputWriter> writer,
  BendersSolutionFn benders_solution_fn,
  IterationFn iteration_fn):
    data_(data),
    criteria_vector_for_each_iteration_(criteria_vector_for_each_iteration),
    relevant_iteration_data_(relevant_iteration_data),
    writer_(std::move(writer)),
    benders_solution_fn_(std::move(benders_solution_fn)),
    iteration_fn_(std::move(iteration_fn))
{
}

CriteriaCurrentIterationData BendersOuterLoopManager::GetOuterLoopData() const
{
    return data_.criteria_current_iteration_data;
}

std::vector<double> BendersOuterLoopManager::GetOuterLoopCriterionAtBestBenders() const
{
    return ((criteria_vector_for_each_iteration_.empty())
              ? std::vector<double>()
              : criteria_vector_for_each_iteration_[data_.best_it - 1]);
}

void BendersOuterLoopManager::UpdateOuterLoopSolution()
{
    outer_loop_solution_data_ = benders_solution_fn_();
    outer_loop_solution_data_.best_it = data_.criteria_current_iteration_data.benders_num_run;
}

Output::SolutionData BendersOuterLoopManager::GetOuterLoopSolution() const
{
    return outer_loop_solution_data_;
}

void BendersOuterLoopManager::SaveOuterLoopSolutionInOutputFile() const
{
    writer_->write_solution(GetOuterLoopSolution());
    writer_->dump();
}

void BendersOuterLoopManager::SaveCurrentOuterLoopIterationInOutputFile() const
{
    const auto& LastWorkerMasterData = relevant_iteration_data_.last;
    if (LastWorkerMasterData._valid)
    {
        writer_->write_iteration(iteration_fn_(LastWorkerMasterData),
                                 data_.criteria_current_iteration_data.benders_num_run);
        writer_->dump();
    }
}

void BendersOuterLoopManager::SetBilevelBestub(double bilevel_best_ub)
{
    data_.criteria_current_iteration_data.outer_loop_bilevel_best_ub = bilevel_best_ub;
}
