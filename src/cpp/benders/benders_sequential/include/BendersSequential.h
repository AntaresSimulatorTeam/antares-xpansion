#pragma once

#include "BendersBase.h"
#include "common.h"
#include "core/ILogger.h"

/*!
 * \class BendersSequential
 * \brief Class use run the benders algorithm in sequential
 */
class BendersSequential : public BendersBase {
 public:
  explicit BendersSequential(BendersBaseOptions const &options, Logger logger,
                             Writer writer);
  virtual ~BendersSequential() = default;
  virtual void launch();
  virtual void build_cut();
  virtual void initialize_problems();

 protected:
  virtual void free();
  virtual void run();

 private:
  [[nodiscard]] bool shouldParallelize() const final { return true; }
};
