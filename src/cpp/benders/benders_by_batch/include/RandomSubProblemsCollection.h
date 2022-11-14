#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_SUBPROBLEMSCOLLECTION_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_SUBPROBLEMSCOLLECTION_H_
#include <stdexcept>

#include "SubproblemWorker.h"
struct Batch {
  std::vector<std::string> sub_problems_names;
  unsigned id;
};
class BatchCollection {
 private:
  std::vector<Batch> batch_collections_;
  unsigned number_of_batch_;

 public:
  BatchCollection(SubproblemsMapPtr sub_problems_map_ptr,
                  unsigned size_of_sub_problems_collection);
  class InvalidSizeOfSubPrblemCollection : public std::runtime_error {
   public:
    explicit InvalidSizeOfSubPrblemCollection(
        unsigned size, unsigned sub_problems_map_ptr_size)
        : std::runtime_error(std::string(
              "Bacth size: " + std::to_string(size) +
              " must be less or equal than number of sub problems: " +
              std::to_string(sub_problems_map_ptr_size))) {}
  };
  size_t size() const { return batch_collections_.size(); }
  std::vector<Batch> BatchCollections() const { return batch_collections_; }
  unsigned NumberOfBatch() const { return number_of_batch_; }
};
#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_SUBPROBLEMSCOLLECTION_H_
