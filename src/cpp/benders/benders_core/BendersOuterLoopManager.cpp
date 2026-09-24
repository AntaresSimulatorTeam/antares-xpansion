#include "antares-xpansion/benders/benders_core/BendersOuterLoopManager.h"

#include "antares-xpansion/benders/output/OutputWriter.h"

BendersOuterLoopManager::BendersOuterLoopManager(
  CurrentIterationData& data,
  const BendersRelevantIterationsData& relevant_iteration_data,
  std::shared_ptr<Output::OutputWriter> writer,
  BendersSolutionFn benders_solution_fn,
  IterationFn iteration_fn):
    data_(data),
    relevant_iteration_data_(relevant_iteration_data),
    writer_(std::move(writer)),
    benders_solution_fn_(std::move(benders_solution_fn)),
    iteration_fn_(std::move(iteration_fn))
{
}

CriteriaCurrentIterationData BendersOuterLoopManager::GetOuterLoopData() const
{
    return data_.criteria;
}

std::vector<double> BendersOuterLoopManager::GetOuterLoopCriterionAtBestBenders() const
{
    return ((criteria_vector_for_each_iteration_.empty())
              ? std::vector<double>()
              : criteria_vector_for_each_iteration_[data_.control.best_it - 1]);
}

void BendersOuterLoopManager::UpdateOuterLoopSolution()
{
    outer_loop_solution_data_ = benders_solution_fn_();
    outer_loop_solution_data_.best_it = data_.criteria.benders_num_run;
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
                                 data_.criteria.benders_num_run);
        writer_->dump();
    }
}

void BendersOuterLoopManager::SetBilevelBestub(double bilevel_best_ub)
{
    data_.criteria.outer_loop_bilevel_best_ub = bilevel_best_ub;
}

void BendersOuterLoopManager::SetCriterionComputationInputs(
  const Benders::Criterion::CriterionInputData& criterion_input_data)
{
    criterion_computation_ = Benders::Criterion::CriterionComputation(criterion_input_data);
}

Benders::Criterion::CriterionComputation& BendersOuterLoopManager::GetCriterionComputation()
{
    return criterion_computation_;
}

const Benders::Criterion::CriterionComputation& BendersOuterLoopManager::GetCriterionComputation()
  const
{
    return criterion_computation_;
}

void BendersOuterLoopManager::PushCriteriaForIteration(const std::vector<double>& criteria)
{
    criteria_vector_for_each_iteration_.push_back(criteria);
}

void BendersOuterLoopManager::ClearCriteriaHistory()
{
    criteria_vector_for_each_iteration_.clear();
}

void BendersOuterLoopManager::UpdateMaxCriterionArea()
{
    auto criteria_begin = data_.criteria.criteria.cbegin();
    auto criteria_end = data_.criteria.criteria.cend();
    auto max_criterion_it = std::max_element(criteria_begin, criteria_end);
    if (max_criterion_it != criteria_end)
    {
        data_.criteria.max_criterion = *max_criterion_it;
        auto max_criterion_index = std::distance(criteria_begin, max_criterion_it);
        data_.criteria.max_criterion_area = criterion_computation_.getCriterionInputData()
                                              .Criteria()[max_criterion_index]
                                              .Pattern()
                                              .GetBody();
    }
}
