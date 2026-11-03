#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "LoggerStub.h"
#include "RecordingSolver.h"
#include "antares-xpansion/benders/benders_core/SubproblemBasisCache.h"

namespace {
Logger MakeLogger()
{
    return std::make_shared<Xpansion::Test::LoggerNOOPStub>();
}
}  // namespace

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
    target_solver.nrows = 2;
    target_solver.ncols = 2;
    EXPECT_TRUE(cache.TryRestore("sub1", target_solver, MakeLogger()));

    EXPECT_EQ(target_solver.set_basis_calls, 1);
    EXPECT_EQ(target_solver.set_basis_rstatus, std::vector<int>({1, 2}));
    EXPECT_EQ(target_solver.set_basis_cstatus, std::vector<int>({3, 4}));
}

TEST(SubproblemBasisCacheTest, TryRestoreOnUnknownNameReturnsFalseAndDoesNotCallSetBasis)
{
    SubproblemBasisCache cache;
    RecordingSolver solver;

    EXPECT_FALSE(cache.TryRestore("never_stored", solver, MakeLogger()));
    EXPECT_EQ(solver.set_basis_calls, 0);
}

// Meant to be run under ThreadSanitizer: concurrent Store (which inserts, hence may rehash) and
// TryRestore used to race when BendersSequential solves subproblems with std::execution::par.
TEST(SubproblemBasisCacheTest, ConcurrentStoreAndTryRestoreOnDifferentNames)
{
    constexpr int n_threads = 8;
    constexpr int n_names_per_thread = 200;
    SubproblemBasisCache cache;
    std::vector<std::thread> threads;
    std::vector<int> restored(n_threads, 0);

    for (int t = 0; t < n_threads; ++t)
    {
        threads.emplace_back(
          [&cache, &restored, t]
          {
              RecordingSolver solver;
              solver.nrows = 2;
              solver.ncols = 2;
              solver.basis_rstatus_out = {1, 2};
              solver.basis_cstatus_out = {3, 4};
              for (int i = 0; i < n_names_per_thread; ++i)
              {
                  const auto own_name = "sub_" + std::to_string(t) + "_" + std::to_string(i);
                  const auto other_name = "sub_" + std::to_string((t + 1) % n_threads) + "_"
                                          + std::to_string(i);
                  cache.TryRestore(other_name, solver, MakeLogger());
                  cache.Store(own_name, solver);
                  restored[t] += cache.TryRestore(own_name, solver, MakeLogger());
              }
          });
    }
    for (auto& thread: threads)
    {
        thread.join();
    }
    for (int count: restored)
    {
        EXPECT_EQ(count, n_names_per_thread);
    }
}
