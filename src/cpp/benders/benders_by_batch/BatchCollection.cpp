#include "antares-xpansion/benders/benders_by_batch/BatchCollection.h"

#include <cmath>
#include <iostream>
BatchCollection::BatchCollection(
    const std::vector<std::string>& sub_problem_names, size_t batch_size,
    Logger logger)
    : sub_problem_names_(sub_problem_names),
      sub_problems_number_(sub_problem_names.size()),
      batch_size_(batch_size),
      logger_(std::move(logger)) {}

void BatchCollection::BuildBatches() {
  if (batch_size_ > sub_problems_number_) {
    logger_->display_message(
        std::string("batch_size(") + std::to_string(batch_size_) +
            ") can not be greater than number of subproblems (" +
            std::to_string(sub_problems_number_) + ")",
        LogUtils::LOGLEVEL::WARNING, "Benders By batch");
    logger_->display_message(
        std::string("Setting batch_size = number of subproblems(") +
        std::to_string(sub_problems_number_) +
        ")\nWhich means that there is only one batch!");
  }
  number_of_batch_ = std::ceil(double(sub_problems_number_) / batch_size_);
  for (auto id = 0; id < number_of_batch_ - 1; id++) {
    Batch b;
    b.id = id;
    b.sub_problem_names.insert(
        b.sub_problem_names.end(),
        sub_problem_names_.begin() + batch_size_ * id,
        sub_problem_names_.begin() + batch_size_ * (id + 1));
    batch_collections_.push_back(b);
  }
  Batch last;
  last.id = number_of_batch_ - 1;
  last.sub_problem_names.insert(
      last.sub_problem_names.end(),
      sub_problem_names_.begin() + (number_of_batch_ - 1) * batch_size_,
      sub_problem_names_.end());
  batch_collections_.push_back(last);
}
