#include "antares-xpansion/benders/benders_by_batch/BendersSubProblemsManagerByBatch.hxx"
#include "antares-xpansion/benders/plugins/NoOperationPlugin.h"
#include "LoggerStub.h"
#include "WriterStub.h"
#include "gtest/gtest.h"

class BendersSubProblemsManagerByBatchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SolverBaseOptions solver_opts;
        solver_opts.SOLVER_NAME = "COIN";
        options_ = std::make_unique<BendersBaseOptions>(solver_opts);
        plugin_ = std::make_shared<NoOperationPlugin>();
        logger_ = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer_ = std::make_shared<Xpansion::Test::WriterNOOPStub>();
    }

    BendersSubProblemsManagerByBatch MakeManager()
    {
        return BendersSubProblemsManagerByBatch(data_,
                                                *options_,
                                                plugin_,
                                                logger_,
                                                solver_log_manager_,
                                                writer_,
                                                coupling_map_);
    }

    CurrentIterationData data_;
    std::unique_ptr<BendersBaseOptions> options_;
    std::shared_ptr<BendersPlugin> plugin_;
    Logger logger_;
    SolverLogManager solver_log_manager_;
    std::shared_ptr<Output::OutputWriter> writer_;
    CouplingMap coupling_map_;
};

TEST_F(BendersSubProblemsManagerByBatchTest, FastBeginHookThrowsOnSubproblemMissingFromSubproblemMap)
{
    auto manager = MakeManager();
    const std::vector<std::string> batch = {"missing.mps"};

    auto hook = manager.MakeFastBeginHook(batch);

    EXPECT_THROW(hook(), std::runtime_error);
}

TEST_F(BendersSubProblemsManagerByBatchTest, CacheBeginHookThrowsOnSubproblemMissingFromCouplingMap)
{
    auto manager = MakeManager();
    const std::vector<std::string> batch = {"missing.mps"};

    auto hook = manager.MakeCacheBeginHook(batch);

    EXPECT_THROW(hook(), std::runtime_error);
}

TEST_F(BendersSubProblemsManagerByBatchTest, CacheBeginHookReturnsEntriesPresentInCouplingMap)
{
    coupling_map_["present.mps"] = {{"var", 0}};
    auto manager = MakeManager();
    const std::vector<std::string> batch = {"present.mps"};

    auto result = manager.MakeCacheBeginHook(batch)();

    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].first, "present.mps");
}

TEST_F(BendersSubProblemsManagerByBatchTest, DistributeKeepsOnlyTheSubproblemsOfTheRankRoundRobin)
{
    options_->CACHE_PROBLEMS = CacheProblems::PER_SUB; // no worker is built from a file
    const std::vector<std::string> names = {"a", "b", "c", "d", "e", "f", "g"};
    constexpr int world_size = 3;

    BatchCollection reference(names, 3, logger_);
    reference.BuildBatches(world_size);
    std::vector<std::string> in_batch_order;
    for (const auto& batch: reference.BatchCollections())
    {
        in_batch_order.insert(in_batch_order.end(),
                              batch.sub_problem_names.begin(),
                              batch.sub_problem_names.end());
    }
    ASSERT_EQ(in_batch_order.size(), names.size());

    for (int rank = 0; rank < world_size; ++rank)
    {
        auto manager = MakeManager();
        BatchCollection collection(names, 3, logger_);
        collection.BuildBatches(world_size);

        manager.DistributeSubproblems(collection, rank, world_size);

        std::vector<std::string> expected;
        for (std::size_t i = rank; i < in_batch_order.size(); i += world_size)
        {
            expected.push_back(in_batch_order[i]);
        }
        std::vector<std::string> kept;
        for (const auto& batch: collection.BatchCollections())
        {
            kept.insert(kept.end(), batch.sub_problem_names.begin(), batch.sub_problem_names.end());
        }
        EXPECT_EQ(kept, expected) << "rank " << rank;
        EXPECT_EQ(manager.GetSubProblemNames(), expected) << "rank " << rank;
    }
}
