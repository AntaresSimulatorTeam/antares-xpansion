#include "BatchCollection.h"
#include "BendersByBatch.h"
#include "gtest/gtest.h"

class BatchCollectionTest : public ::testing::Test {
 public:
  const std::vector<std::string> sub_problems_name_list = {"P1", "P2", "P3"};
  const std::vector<std::string> sub_problems_name_list_4 = {"P1", "P2", "P3",
                                                             "P4"};

 protected:
};

TEST_F(BatchCollectionTest,
       NumberOfCollectionCanNotBeGreaterThanNumberOfSubProblems) {
  auto batch_collection = BatchCollection(sub_problems_name_list,
                                          sub_problems_name_list.size() + 1);
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