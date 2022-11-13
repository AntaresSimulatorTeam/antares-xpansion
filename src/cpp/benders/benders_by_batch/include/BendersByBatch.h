#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#include "BendersBase.h"

class BendersByBatch : public BendersBase {
 private:
  /* data */
 public:
  BendersByBatch(BendersBaseOptions const &options, Logger logger,
                 Writer writer);
  ~BendersByBatch() = default;
  void launch() override;
  void free() override;
  void run() override;
  void initialize_problems() override;
};

#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
