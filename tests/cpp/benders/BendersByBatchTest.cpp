#include <numeric>

#include "BatchCollection.h"
#include "BendersByBatch.h"
#include "RandomBatchShuffler.h"
#include "gtest/gtest.h"

class BatchCollectionTest : public ::testing::Test {
 public:
  const std::vector<std::string> sub_problems_name_list = {"P1", "P2", "P3"};
  const std::vector<std::string> sub_problems_name_list_4 = {"P1", "P2", "P3",
                                                             "P4"};
  const std::vector<std::string> sub_problems_name_list_5 = {"P1", "P2", "P3",
                                                             "P4", "P5"};

 protected:
};

TEST_F(BatchCollectionTest,
       NumberOfCollectionCanNotBeGreaterThanNumberOfSubProblems) {
  auto sub_problems_name_list_size = sub_problems_name_list.size();
  auto batch_size = sub_problems_name_list_size + 1;
  try {
    auto batch_collection = BatchCollection(sub_problems_name_list, batch_size);
  } catch (const BatchCollection::InvalidSizeOfSubProblemCollection& err) {
    auto expected = (std::string("Bacth size: ") + std::to_string(batch_size) +
                     " must be less or equal than number of sub problems: " +
                     std::to_string(sub_problems_name_list_size));
    ASSERT_STREQ(err.what(), expected.c_str());
  }
}
TEST_F(
    BatchCollectionTest,
    IfNumberOfSubProblemIsEqualToBatchSizeThanBatchCollectionSizeIsEqualTo1) {
  auto batch_collection =
      BatchCollection(sub_problems_name_list, sub_problems_name_list.size());
  ASSERT_EQ(batch_collection.size(), 1);
}
TEST_F(BatchCollectionTest, NumberOfSubProblemIsEqualToTheSumOfBatchSize) {
  auto batch_collection =
      BatchCollection(sub_problems_name_list, sub_problems_name_list.size());

  auto sum_batch_size = 0;
  for (const auto& batch : batch_collection.BatchCollections()) {
    sum_batch_size += batch.sub_problem_names.size();
  }
  ASSERT_EQ(sum_batch_size, sub_problems_name_list.size());
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs3AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problems_name_list, 2);

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs4AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problems_name_list_4, 2);

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs5AndBatchSize2Then_P2_isInBatch1AndP3IsInBatch2) {
  auto batch_collection = BatchCollection(sub_problems_name_list_5, 2);

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
TEST_F(RandomBatchShufflerTest, GetRandomBatchPermutation) {
  unsigned number_of_batch(10);
  auto batch_suffler = RandomBatchShuffler(number_of_batch);
  const auto random_batch_permutation = batch_suffler.GetRandomBatchOrder();
  std::vector<unsigned> not_expected_vec(number_of_batch);
  std::iota(not_expected_vec.begin(), not_expected_vec.end(), 0);

  ASSERT_FALSE(not_expected_vec == random_batch_permutation);
}