#include "BendersMpiOuterLoop.h"

#include "CustomVector.h"

namespace Outerloop {

BendersMpiOuterLoop::BendersMpiOuterLoop(
    const BendersBaseOptions& options, Logger logger, Writer writer,
    mpi::environment& env, mpi::communicator& world,
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver)
    : BendersMpi(options, logger, writer, env, world, mathLoggerDriver) {}

void BendersMpiOuterLoop::SolveSubproblem(
    SubProblemDataMap& subproblem_data_map,
    PlainData::SubProblemData& subproblem_data, const std::string& name,
    const std::shared_ptr<SubproblemWorker>& worker) {
  BendersBase::SolveSubproblem(subproblem_data_map, subproblem_data, name,
                               worker);

  std::vector<double> solution;
  worker->get_solution(solution);
  criterions_computation_->ComputeOuterLoopCriterion(
      SubproblemWeight(_data.nsubproblem, name), solution,
      subproblem_data.outer_loop_criterions,
      subproblem_data.outer_loop_patterns_values);
}



void BendersMpiOuterLoop::UpdateOuterLoopMaxCriterionArea() {
  auto criterions_begin =
      _data.outer_loop_current_iteration_data.outer_loop_criterion.cbegin();
  auto criterions_end =
      _data.outer_loop_current_iteration_data.outer_loop_criterion.cend();
  auto max_criterion_it = std::max_element(criterions_begin, criterions_end);
  _data.outer_loop_current_iteration_data.max_criterion = *max_criterion_it;
  auto max_criterion_index = std::distance(criterions_begin, max_criterion_it);
  _data.outer_loop_current_iteration_data.max_criterion_area =
      criterions_computation_->getOuterLoopInputData()
          .OuterLoopData()[max_criterion_index]
          .Pattern()
          .GetBody();
}

void BendersMpiOuterLoop::InitializeProblems() {
  BendersMpi::InitializeProblems();
  init_problems_ = false;
}

void BendersMpiOuterLoop::ComputeSubproblemsContributionToOuterLoopCriterion(
    const SubProblemDataMap& subproblem_data_map) {
  const auto vars_size = criterions_computation_->getVarIndices().size();
  std::vector<double> outer_loop_criterion_per_sub_problem_per_pattern(
      vars_size, {});
  _data.outer_loop_current_iteration_data.outer_loop_criterion.resize(vars_size,
                                                                      0.);
  std::vector<double> outer_loop_patterns_values_per_sub_problem_per_pattern(
      vars_size, {});
  _data.outer_loop_current_iteration_data.outer_loop_patterns_values.resize(
      vars_size, 0.);

  for (const auto& [subproblem_name, subproblem_data] : subproblem_data_map) {
    AddVectors<double>(outer_loop_criterion_per_sub_problem_per_pattern,
                       subproblem_data.outer_loop_criterions);
    AddVectors<double>(outer_loop_patterns_values_per_sub_problem_per_pattern,
                       subproblem_data.outer_loop_patterns_values);
  }

  Reduce(outer_loop_criterion_per_sub_problem_per_pattern,
         _data.outer_loop_current_iteration_data.outer_loop_criterion,
         std::plus<double>(), rank_0);
  Reduce(outer_loop_patterns_values_per_sub_problem_per_pattern,
         _data.outer_loop_current_iteration_data.outer_loop_patterns_values,
         std::plus<double>(), rank_0);
}

void BendersMpiOuterLoop::GatherCuts(
    const SubProblemDataMap& subproblem_data_map, const Timer& walltime) {
  BendersMpi::GatherCuts(subproblem_data_map, walltime);

  ComputeSubproblemsContributionToOuterLoopCriterion(subproblem_data_map);
  if (_world.rank() == rank_0) {
    outer_loop_criterion_.push_back(
        _data.outer_loop_current_iteration_data.outer_loop_criterion);
    UpdateOuterLoopMaxCriterionArea();
  }
}

void BendersMpiOuterLoop::launch() {
  ++_data.outer_loop_current_iteration_data.benders_num_run;
  BendersMpi::launch();
}
}  // namespace Outerloop