#include "antares-xpansion/benders/benders_by_batch/RandomBatchShuffler.h"

std::vector<unsigned> RandomBatchShuffler::GetCyclicBatchOrder(
    unsigned batch_counter) const {
  std::vector<unsigned> vec(number_of_batch_);
  for (auto count = 0; count < number_of_batch_; count++) {
    vec[count] = (batch_counter + count) % number_of_batch_;
  }
  return vec;
}