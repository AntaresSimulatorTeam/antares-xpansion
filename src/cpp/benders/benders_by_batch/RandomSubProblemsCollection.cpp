#include "RandomSubProblemsCollection.h"

#include <cmath>
BatchCollection::BatchCollection(SubproblemsMapPtr sub_problems_map_ptr,
                                 unsigned batch_size) {
  const auto sub_problems_number = sub_problems_map_ptr.size();

  if (batch_size > sub_problems_number) {
    throw BatchCollection::InvalidSizeOfSubPrblemCollection(
        batch_size, sub_problems_number);
  }
  if (batch_size == sub_problems_number) {
    number_of_batch_ = 1;
    std::vector<std::string> sub_problems_names;
    for (const auto &[name, worker] : sub_problems_map_ptr) {
      sub_problems_names.push_back(name);
    }
    batch_collections_.push_back({sub_problems_names, 0});
  } else {
    number_of_batch_ =
        std::ceil(double(sub_problems_map_ptr.size()) / batch_size);
    // for (auto id = 0; id < number_of_batch_; id++) {
    //   Batch b = {}
    // }
  }
}
