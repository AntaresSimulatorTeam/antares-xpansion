#pragma once

#include <memory>

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/ICommunicationStrategy.h"
#include "antares-xpansion/benders/benders_core/IOuterLoopStrategy.h"
#include "antares-xpansion/benders/benders_core/ISubproblemSolver.h"

class BendersAlgorithm
{
public:
    BendersAlgorithm(std::shared_ptr<ICommunicationStrategy> comm,
                     std::shared_ptr<ISubproblemSolver> solver,
                     std::shared_ptr<IOuterLoopStrategy> outer_loop,
                     std::shared_ptr<BendersBase> benders_base);

    void Run();

    // The core Benders loop that can be called by OuterLoop or directly
    void MasterLoop();

    void set_outer_loop(std::shared_ptr<IOuterLoopStrategy> outer_loop)
    {
        outer_loop_ = std::move(outer_loop);
    }

    std::shared_ptr<BendersBase> get_benders_base() const
    {
        return benders_base_;
    }

private:
    void init();
    void solve_master();
    void solve_subproblems();
    void update_bounds();
    void check_convergence();
    void finalize();

    std::shared_ptr<ICommunicationStrategy> comm_;
    std::shared_ptr<ISubproblemSolver> solver_;
    std::shared_ptr<IOuterLoopStrategy> outer_loop_;
    std::shared_ptr<BendersBase> benders_base_;
};
