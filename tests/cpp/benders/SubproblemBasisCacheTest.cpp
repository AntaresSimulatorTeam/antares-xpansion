#include <memory>

#include <gtest/gtest.h>

#include "RecordingSolver.h"
#include "antares-xpansion/benders/benders_core/SubproblemBasisCache.h"

TEST(SubproblemBasisCacheTest, StoreThenTryRestoreRoundTripsTheBasis)
{
    RecordingSolver source_solver;
    source_solver.nrows = 2;
    source_solver.ncols = 2;
    source_solver.basis_rstatus_out = {1, 2};
    source_solver.basis_cstatus_out = {3, 4};

    SubproblemBasisCache cache;
    cache.Store("sub1", source_solver);

    RecordingSolver target_solver;
    EXPECT_TRUE(cache.TryRestore("sub1", target_solver));

    EXPECT_EQ(target_solver.set_basis_calls, 1);
    EXPECT_EQ(target_solver.set_basis_rstatus, std::vector<int>({1, 2}));
    EXPECT_EQ(target_solver.set_basis_cstatus, std::vector<int>({3, 4}));
}

TEST(SubproblemBasisCacheTest, TryRestoreOnUnknownNameReturnsFalseAndDoesNotCallSetBasis)
{
    SubproblemBasisCache cache;
    RecordingSolver solver;

    EXPECT_FALSE(cache.TryRestore("never_stored", solver));
    EXPECT_EQ(solver.set_basis_calls, 0);
}
