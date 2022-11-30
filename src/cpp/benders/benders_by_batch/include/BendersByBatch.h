#ifndef SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#define SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
#include "BatchCollection.h"
#include "BendersBase.h"

class BendersByBatch : public BendersBase {
 private:
  [[nodiscard]] bool shouldParallelize() const final { return true; }
  std::vector<unsigned> random_batch_permutation_;

 public:
  BendersByBatch(BendersBaseOptions const &options, Logger logger,
                 Writer writer);
  ~BendersByBatch() override = default;
  void launch() override;
  void free() override;
  void run() override;
  void initialize_problems() override;
  void build_cut(const std::vector<std::string> &batch_sub_problems,
                 double *sum);

 private:
  void getSubproblemCut(SubproblemCutPackage &subproblem_cut_package,
                        const std::vector<std::string> &batch_sub_problems,
                        double *sum) const;
};

#endif  // SRC_CPP_BENDERS_BENDERS_BY_BATCH_INCLUDE_BENDERSBYBATCH_H_
