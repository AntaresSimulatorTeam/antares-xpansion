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
        solve_master();
        solve_subproblems();
        update_bounds();
        check_convergence();
    }
    finalize();
}

void BendersAlgorithm::init()
{
    benders_base_->init_data();
    benders_base_->OpenCsvFile();
    benders_base_->mathLoggerDriverWriteheader();
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
}
