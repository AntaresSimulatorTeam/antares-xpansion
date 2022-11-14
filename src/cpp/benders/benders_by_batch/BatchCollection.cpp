#include <cmath>

#include "BatchCollection.h"
BatchCollection::BatchCollection(StrVector sub_problem_names,
                                 unsigned batch_size) {
  const auto sub_problems_number = sub_problem_names.size();

  if (batch_size > sub_problems_number) {
    throw BatchCollection::InvalidSizeOfSubPrblemCollection(
        batch_size, sub_problems_number);
  }
  if (batch_size == sub_problems_number) {
    number_of_batch_ = 1;
    batch_collections_.push_back({sub_problem_names, 0});
  } else {
    number_of_batch_ = std::ceil(double(sub_problems_number) / batch_size);
    // for (auto id = 0; id < number_of_batch_; id++) {
    //   Batch b = {}
    // }
  }
}
