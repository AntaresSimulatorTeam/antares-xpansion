#include "antares-xpansion/benders/benders_core/BatchSubproblemSolver.h"

#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"

void BatchSubproblemSolver::Solve(CurrentIterationData& data)
{
    SeparationLoop();
}

void BatchSubproblemSolver::SeparationLoop()
{
    auto benders_by_batch = dynamic_cast<BendersByBatch*>(benders_base_.get());
    if (!benders_by_batch)
    {
        benders_base_->BuildCut();
        return;
    }

    benders_by_batch->misprice_ = true;
    benders_by_batch->first_unsolved_batch_ = 0;
    benders_by_batch->batch_counter_ = 0;
    while (benders_by_batch->misprice_
           && benders_by_batch->batch_counter_ < benders_by_batch->number_of_batch_)
    {
        benders_by_batch->_data.it++;
        benders_by_batch->ResetSimplexIterationsBounds();

        benders_by_batch->_logger->log_at_initialization(
          benders_by_batch->_data.it + benders_by_batch->GetNumIterationsBeforeRestart());
        if (benders_by_batch->Rank() == benders_by_batch->rank_0)
        {
            benders_by_batch->ComputeXCut();
        }
        benders_by_batch->BroadcastXCut();
        benders_by_batch->_logger->log_iteration_candidates(
          benders_by_batch->bendersDataToLogData(benders_by_batch->_data));
        benders_by_batch->UpdateRemainingEpsilon();
        benders_by_batch->_data.number_of_subproblem_solved = 0;
        benders_by_batch->SolveBatches();

        if (benders_by_batch->Rank() == benders_by_batch->rank_0)
        {
            benders_by_batch->criteria_vector_for_each_iteration_.push_back(
              benders_by_batch->_data.criteria_current_iteration_data.criteria);
            benders_by_batch->UpdateTrace();
            benders_by_batch->SaveCurrentBendersData();
        }
        benders_by_batch->ClearCurrentIterationCutTrace();
    }
}
