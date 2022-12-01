#include "BatchCollection.h"

#include <cmath>
BatchCollection::BatchCollection(
    const std::vector<std::string>& sub_problem_names, size_t batch_size,
    Logger logger)
    : logger_(std::move(logger)) {
  const auto sub_problems_number = sub_problem_names.size();

  if (batch_size > sub_problems_number) {
    logger_->display_message(
        std::string("Warning: batch_size(") + std::to_string(batch_size) +
        ") can not be greater than number of subproblems (" +
        std::to_string(sub_problems_number) + ")");
    logger_->display_message(
        std::string("Setting batch_size = number of subproblems(") +
        std::to_string(sub_problems_number) +
        ")\nWhich means that there is only one batch!\n");
  }
  number_of_batch_ = std::ceil(double(sub_problems_number) / batch_size);
  for (auto id = 0; id < number_of_batch_ - 1; id++) {
    Batch b;
    b.id = id;
    b.sub_problem_names.insert(
        b.sub_problem_names.end(), sub_problem_names.begin() + batch_size * id,
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
