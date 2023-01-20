#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_RANDOMBATCHSHUFFLER_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_RANDOMBATCHSHUFFLER_H_

#include <vector>

class RandomBatchShuffler {
 private:
  unsigned number_of_batch_;

 public:
  explicit RandomBatchShuffler(unsigned number_of_batch)
      : number_of_batch_(number_of_batch) {}

  std::vector<unsigned> GetCyclicBatchOrder(unsigned batch_counter) const;
};
#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_RANDOMBATCHSHUFFLER_H_
