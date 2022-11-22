#include "BatchCollection.h"

#include <cmath>
BatchCollection::BatchCollection(
    const std::vector<std::string>& sub_problem_names, size_t batch_size) {
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
    for (auto id = 0; id < number_of_batch_ - 1; id++) {
      Batch b;
      b.id = id;
      b.sub_problem_names.insert(
          b.sub_problem_names.end(),
          sub_problem_names.begin() + batch_size * id,
          sub_problem_names.begin() + batch_size * (id + 1));
      batch_collections_.push_back(b);
    }
    Batch last;
    last.id = number_of_batch_ - 1;
    last.sub_problem_names.insert(
        last.sub_problem_names.end(),
        sub_problem_names.begin() + (number_of_batch_ - 1) * batch_size,
        sub_problem_names.end());
    batch_collections_.push_back(last);
  }
}
