#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#include <stdexcept>
#include <string>
#include <vector>

struct Batch {
  std::vector<std::string> sub_problem_names;
  unsigned id;
};
class BatchCollection {
 private:
  std::vector<Batch> batch_collections_;
  unsigned number_of_batch_;

 public:
  BatchCollection(const std::vector<std::string>& sub_problem_names,
                  unsigned size_of_sub_problems_collection);
  class InvalidSizeOfSubPrblemCollection : public std::runtime_error {
   public:
    explicit InvalidSizeOfSubPrblemCollection(unsigned size,
                                              unsigned sub_problems_names_size)
        : std::runtime_error(std::string(
              "Bacth size: " + std::to_string(size) +
              " must be less or equal than number of sub problems: " +
              std::to_string(sub_problems_names_size))) {}
  };
  size_t size() const { return batch_collections_.size(); }
  std::vector<Batch> BatchCollections() const { return batch_collections_; }
  Batch GetBatchFromId(unsigned id) const { return batch_collections_[id]; }
  unsigned NumberOfBatch() const { return number_of_batch_; }
};
#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
