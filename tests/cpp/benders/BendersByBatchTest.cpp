#include <numeric>

#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"
#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"
#include "LogPrefixManip.h"
#include "antares-xpansion/benders/benders_by_batch/RandomBatchShuffler.h"
#include "gtest/gtest.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "antares-xpansion/benders/logger/User.h"

class BatchCollectionTest : public ::testing::Test {
 public:
  const std::vector<std::string> sub_problems_name_list = {"P1", "P2", "P3"};
  const std::vector<std::string> sub_problems_name_list_4 = {"P1", "P2", "P3",
                                                             "P4"};
  const std::vector<std::string> sub_problems_name_list_5 = {"P1", "P2", "P3",
                                                             "P4", "P5"};

 protected:
  Logger logger_;
  Logger std_out_logger;
  void SetUp() override {
    Logger std_out_logger;
    std_out_logger = std::make_shared<xpansion::logger::User>(std::cerr);
    auto master_logger = std::make_shared<xpansion::logger::Master>();
    master_logger->addLogger(std_out_logger);
    logger_ = master_logger;
  }
};

TEST_F(BatchCollectionTest,
       NumberOfCollectionCanNotBeGreaterThanNumberOfSubProblems) {
  auto sub_problems_name_list_size = sub_problems_name_list.size();
  auto batch_size = sub_problems_name_list_size + 1;

  std::stringstream redirectedErrorStream;
  std::streambuf* initialBufferCerr =
      std::cerr.rdbuf(redirectedErrorStream.rdbuf());
  auto expected = std::string(" batch_size(") + std::to_string(batch_size) +
                  ") can not be greater than number of subproblems (" +
                  std::to_string(sub_problems_name_list_size) + ")\n" +
                  "Setting batch_size = number of subproblems(" +
                  std::to_string(sub_problems_name_list_size) +
                  ")\nWhich means that there is only one batch!\n";
  auto batch_collection =
      BatchCollection(sub_problems_name_list, batch_size, logger_);
  batch_collection.BuildBatches();

  std::cerr.rdbuf(initialBufferCerr);
  auto redirectedErrorStreamWithoutLogPrefix =
      RemovePrefixFromMessage(redirectedErrorStream);
  ASSERT_EQ(redirectedErrorStreamWithoutLogPrefix, expected);
}
TEST_F(
    BatchCollectionTest,
    IfNumberOfSubProblemIsEqualToBatchSizeThanBatchCollectionSizeIsEqualTo1) {
  auto batch_collection = BatchCollection(
      sub_problems_name_list, sub_problems_name_list.size(), logger_);
  batch_collection.BuildBatches();

  ASSERT_EQ(batch_collection.size(), 1);
}
TEST_F(BatchCollectionTest, NumberOfSubProblemIsEqualToTheSumOfBatchSize) {
  auto batch_collection = BatchCollection(
      sub_problems_name_list, sub_problems_name_list.size(), logger_);
  batch_collection.BuildBatches();

  auto sum_batch_size = 0;
  for (const auto& batch : batch_collection.BatchCollections()) {
    sum_batch_size += batch.sub_problem_names.size();
  }
  ASSERT_EQ(sum_batch_size, sub_problems_name_list.size());
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs3AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problems_name_list, 2, logger_);
  batch_collection.BuildBatches();

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs4AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problems_name_list_4, 2, logger_);
  batch_collection.BuildBatches();

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs5AndBatchSize2Then_P2_isInBatch1AndP3IsInBatch2) {
  auto batch_collection = BatchCollection(sub_problems_name_list_5, 2, logger_);
  batch_collection.BuildBatches();

  ASSERT_EQ(batch_collection.NumberOfBatch(), 3);
  const auto vec_batch0 = batch_collection.GetBatchFromId(0).sub_problem_names;
  const auto vec_batch1 = batch_collection.GetBatchFromId(1).sub_problem_names;
  const auto vec_batch2 = batch_collection.GetBatchFromId(2).sub_problem_names;

  const std::vector<std::string> expected_vec_batch0_names = {"P1", "P2"};
  const std::vector<std::string> expected_vec_batch1_names = {"P3", "P4"};
  const std::vector<std::string> expected_vec_batch2_names = {"P5"};
  ASSERT_EQ(expected_vec_batch0_names, vec_batch0);
  ASSERT_EQ(expected_vec_batch1_names, vec_batch1);
  ASSERT_EQ(expected_vec_batch2_names, vec_batch2);
}

class RandomBatchShufflerTest : public ::testing::Test {};

TEST_F(RandomBatchShufflerTest, GetCyclicBatchPermutation) {
  unsigned number_of_batch(10);
  auto batch_suffler = RandomBatchShuffler(number_of_batch);
  const auto random_batch_permutation = batch_suffler.GetCyclicBatchOrder(5);
  std::vector<unsigned> expected_vec = {5, 6, 7, 8, 9, 0, 1, 2, 3, 4};

  ASSERT_TRUE(expected_vec == random_batch_permutation);
}