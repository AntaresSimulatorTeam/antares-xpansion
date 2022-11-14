#include "RandomBatchShuffler.h"

#include <algorithm>
#include <numeric>
#include <random>

std::vector<unsigned> RandomBatchShuffler::GetRandomBatchOrder() const {
  std::vector<unsigned> vec(number_of_batch_);
  std::iota(vec.begin(), vec.end(), 0);
  auto rng = std::default_random_engine{};
  std::shuffle(vec.begin(), vec.end(), rng);
  return vec;
}