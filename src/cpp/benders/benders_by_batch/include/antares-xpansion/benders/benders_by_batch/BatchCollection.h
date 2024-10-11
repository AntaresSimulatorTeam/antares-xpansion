#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#include <boost/serialization/map.hpp>
#include <string>
#include <vector>

#include "ILogger.h"

struct Batch {
  std::vector<std::string> sub_problem_names;
  unsigned id;
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
    ar &sub_problem_names;
    ar &id;
  }
};
class BatchCollection {
 private:
  std::vector<std::string> sub_problem_names_;
  size_t sub_problems_number_;
  size_t batch_size_;
  std::vector<Batch> batch_collections_;
  unsigned number_of_batch_;
  Logger logger_;

 public:
  BatchCollection() = default;
  BatchCollection(const std::vector<std::string> &sub_problem_names,
                  size_t batch_size, Logger logger);
  void BuildBatches();

  void SetLogger(Logger logger) { logger_ = std::move(logger); }
  void SetBatchSize(size_t batch_size) { batch_size_ = batch_size; }
  void SetSubProblemNames(const std::vector<std::string> &sub_problem_names) {
    sub_problem_names_ = sub_problem_names;
    sub_problems_number_ = sub_problem_names.size();
  }
  size_t size() const { return batch_collections_.size(); }
  std::vector<Batch> BatchCollections() const { return batch_collections_; }
  Batch GetBatchFromId(unsigned batch_id) const {
    return batch_collections_[batch_id];
  }
  unsigned NumberOfBatch() const { return number_of_batch_; }
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
    ar &sub_problem_names_;
    ar &sub_problems_number_;
    ar &batch_size_;
    ar &batch_collections_;
    ar &number_of_batch_;
  }
};
#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
