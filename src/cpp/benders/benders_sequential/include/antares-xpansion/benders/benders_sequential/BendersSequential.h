#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/*!
 * \class BendersSequential
 * \brief Class use run the benders algorithm in sequential
 */
class BendersSequential: public BendersBase
{
public:
    explicit BendersSequential(const BendersBaseOptions& options,
                               Logger logger,
                               std::shared_ptr<Output::OutputWriter> writer,
                               std::shared_ptr<MathLoggerDriver> mathLoggerDriver);
    ~BendersSequential() override = default;
    void launch() override;
    void BuildCut() override;
    void InitializeProblems() override;

    std::string BendersName() const override
    {
        return "Sequential";
    }

    void solve_master() override;
    void check_convergence() override;
    Point get_master_x() const override;
    void set_master_x(const Point& x) override;

protected:
    void free() override;
    void Run() override;

    [[nodiscard]] bool shouldParallelize() const final
    {
        return true;
    }

private:
    ArchiveReader reader_;
};
