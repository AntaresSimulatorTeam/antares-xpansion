#pragma once

#include "ArchiveReader.h"
#include "BendersBase.h"
#include "ILogger.h"
#include "common.h"

/*!
 * \class BendersSequential
 * \brief Class use run the benders algorithm in sequential
 */
class BendersSequential : public BendersBase {
 public:
  explicit BendersSequential(
      BendersBaseOptions const &options, Logger logger, Writer writer,
      std::shared_ptr<MathLoggerDriver> mathLoggerDriver);
  virtual ~BendersSequential() = default;
  virtual void launch();
  virtual void BuildCut();
  virtual void InitializeProblems();
  std::string BendersName() const { return "Sequential"; }

  void OuterLoopBilevelChecks() override {}

 protected:
  virtual void free();
  virtual void Run();
  [[nodiscard]] bool shouldParallelize() const final { return true; }

 private:
  ArchiveReader reader_;
};
