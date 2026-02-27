#include "antares-xpansion/benders/benders_core/BendersAlgorithm.h"

BendersAlgorithm::BendersAlgorithm(std::shared_ptr<ICommunicationStrategy> comm,
                                   std::shared_ptr<ISubproblemSolver> solver,
                                   std::shared_ptr<IOuterLoopStrategy> outer_loop,
                                   std::shared_ptr<BendersBase> benders_base):
    comm_(std::move(comm)),
    solver_(std::move(solver)),
    outer_loop_(std::move(outer_loop)),
    benders_base_(std::move(benders_base))
{
}

void BendersAlgorithm::Run()
{
    outer_loop_->Run();
}

void BendersAlgorithm::MasterLoop()
{
    init();
    while (!benders_base_->IsStop())
    {
        ++benders_base_->getCurrentIterationData().it;

        if (comm_->Rank() == 0)
        {
            if (benders_base_->SwitchToIntegerMaster(
                  benders_base_->getCurrentIterationData().is_in_initial_relaxation))
            {
                benders_base_->_logger->LogAtSwitchToInteger();
                benders_base_->ActivateIntegrityConstraints();
                benders_base_->ResetDataPostRelaxation();
            }

            benders_base_->_logger->log_at_initialization(
              benders_base_->getCurrentIterationData().it
              + benders_base_->GetNumIterationsBeforeRestart());
            benders_base_->_logger->display_message("\tSolving master...");
        }

        solve_master();

        if (comm_->Rank() == 0)
        {
            benders_base_->_logger->log_master_solving_duration(
              benders_base_->getCurrentIterationData().timer_master);
        }

        benders_base_->ComputeXCut();

        if (comm_->Rank() == 0)
        {
            benders_base_->_logger->log_iteration_candidates(
              benders_base_->bendersDataToLogData(benders_base_->getCurrentIterationData()));
            benders_base_->_logger->display_message("\tSolving subproblems...");
        }

        solve_subproblems();

        if (comm_->Rank() == 0)
        {
            benders_base_->_logger->LogSubproblemsSolvingWalltime(
              benders_base_->getCurrentIterationData().subproblems_walltime);
        }

        benders_base_->compute_ub();
        update_bounds();
        check_convergence();
    }
    finalize();
}

void BendersAlgorithm::init()
{
    benders_base_->init_data();
    benders_base_->ChecksResumeMode();
    if (benders_base_->is_trace())
    {
        benders_base_->OpenCsvFile();
    }
    benders_base_->mathLoggerDriverWriteheader();
    benders_base_->HandleInitialMasterRelaxation();
}

void BendersAlgorithm::solve_master()
{
    if (comm_->Rank() == 0)
    {
        benders_base_->solve_master();
    }
    // Broadcast solution
    Point x_out = benders_base_->get_master_x();
    comm_->BroadcastXOut(x_out, 0);
    if (comm_->Rank() != 0)
    {
        benders_base_->set_master_x(x_out);
    }
}

void BendersAlgorithm::solve_subproblems()
{
    solver_->Solve(benders_base_->getCurrentIterationData());
}

void BendersAlgorithm::update_bounds()
{
    if (comm_->Rank() == 0)
    {
        benders_base_->update_best_ub();
        benders_base_->UpdateTrace();
        benders_base_->LoggerLogAtIterationEnd();
        benders_base_->SaveCurrentBendersData();
    }
}

void BendersAlgorithm::check_convergence()
{
    if (comm_->Rank() == 0)
    {
        benders_base_->check_convergence();
    }
    bool stop = benders_base_->IsStop();
    comm_->BroadcastStop(stop, 0);
    if (comm_->Rank() != 0)
    {
        benders_base_->set_stop(stop);
    }
}

void BendersAlgorithm::finalize()
{
    benders_base_->CloseCsvFile();
    benders_base_->EndWritingInOutputFile();
    benders_base_->write_basis();
}
