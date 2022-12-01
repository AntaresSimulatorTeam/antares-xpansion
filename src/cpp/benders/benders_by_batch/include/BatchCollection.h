#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
#include <string>
#include <vector>

#include "core/ILogger.h"

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
                  size_t size_of_sub_problems_collection, Logger logger);

  size_t size() const { return batch_collections_.size(); }
  std::vector<Batch> BatchCollections() const { return batch_collections_; }
  Batch GetBatchFromId(unsigned batch_id) const {
    return batch_collections_[batch_id];
  }
  unsigned NumberOfBatch() const { return number_of_batch_; }
  Logger logger_;
};
#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BATCHCOLLECTION_H_
