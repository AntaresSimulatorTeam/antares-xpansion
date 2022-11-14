#include "BendersByBatch.h"
#include "RandomSubProblemsCollection.h"
#include "gtest/gtest.h"

class BatchCollectionTest : public ::testing::Test {
 public:
  SubproblemsMapPtr sub_problem_map_ptr;
  SubproblemsMapPtr sub_problem_map_ptr_4;
  const std::list<std::string> sub_problems_name_list = {"P1", "P2", "P3"};
  const std::list<std::string> sub_problems_name_list_4 = {"P1", "P2", "P3",
                                                           "P4"};

 protected:
  void SetUp() override {
    for (const auto& sub_problem_name : sub_problems_name_list) {
      sub_problem_map_ptr[sub_problem_name] =
          std::make_shared<SubproblemWorker>();
    }
    for (const auto& sub_problem_name : sub_problems_name_list_4) {
      sub_problem_map_ptr_4[sub_problem_name] =
          std::make_shared<SubproblemWorker>();
    }
  }
};

TEST_F(BatchCollectionTest,
       NumberOfCollectionCanNotBeGreaterThanNumberOfSubProblems) {
  auto batch_collection =
      BatchCollection(sub_problem_map_ptr, sub_problem_map_ptr.size() + 1);
}
TEST_F(
    BatchCollectionTest,
    IfNumberOfSubProblemIsEqualToBatchSizeThanBatchCollectionSizeIsEqualTo1) {
  auto batch_collection =
      BatchCollection(sub_problem_map_ptr, sub_problem_map_ptr.size());
  ASSERT_EQ(batch_collection.size(), 1);
}
TEST_F(BatchCollectionTest, NumberOfSubProblemIsEqualToTheSumOfBatchSize) {
  auto batch_collection =
      BatchCollection(sub_problem_map_ptr, sub_problem_map_ptr.size());

  auto sum_batch_size = 0;
  for (const auto& batch : batch_collection.BatchCollections()) {
    sum_batch_size += batch.sub_problems_names.size();
  }
  ASSERT_EQ(sum_batch_size, sub_problem_map_ptr.size());
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs3AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problem_map_ptr, 2);

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}
TEST_F(BatchCollectionTest,
       IfNumberOFSubProblemIs4AndBatchSize2ThenNumberOfBatchIs2) {
  auto batch_collection = BatchCollection(sub_problem_map_ptr_4, 2);

  ASSERT_EQ(batch_collection.NumberOfBatch(), 2);
}