#pragma once

#include "antares-xpansion/archive_handler/ArchiveReader.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "antares-xpansion/benders/benders_core/common.h"

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


 protected:
  virtual void free();
  virtual void Run();
  [[nodiscard]] bool shouldParallelize() const final { return true; }

 private:
  ArchiveReader reader_;
};
