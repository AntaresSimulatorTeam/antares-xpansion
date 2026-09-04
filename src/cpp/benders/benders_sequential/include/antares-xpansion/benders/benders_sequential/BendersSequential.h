#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/*!
 * \class BendersSequential
 * \brief Class use run the benders algorithm in sequential
 *
 * Test-only engine: production runs always use the MPI backends (BendersMpi /
 * BendersByBatch), even for a single process. BendersSequential is kept as a
 * lightweight double for unit tests (no MPI environment required).
 */
class BendersSequential: public BendersBase
{
public:
    explicit BendersSequential(const BendersBaseOptions& options,
                               Logger logger,
                               std::shared_ptr<Output::OutputWriter> writer,
                               std::shared_ptr<MathLoggerDriver> mathLoggerDriver);
    virtual ~BendersSequential() = default;
    virtual void launch();
    virtual void BuildCut();
    virtual void InitializeProblems();

    std::string BendersName() const
    {
        return "Sequential";
    }

protected:
    virtual void free();
    virtual void Run();

private:
    ArchiveReader reader_;
};
