#pragma once

#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/SubproblemCut.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/*!
 * \class GridSearch
 * \brief Class use run the GridSearch algorithm
 */
class GridSearch: public BendersBase
{
public:
    ~GridSearch() override = default;
    GridSearch(const BendersBaseOptions& options,
               Logger logger,
               std::shared_ptr<Output::OutputWriter> writer,
               std::shared_ptr<MathLoggerDriver> mathLoggerDriver);

    void launch() override;

    std::string BendersName() const override
    {
        return "GridSearch";
    }

protected:
    void free() override;
    void Run() override;
    void InitializeProblems() override;

private:
    std::map<std::string, std::vector<Point>> gridPoints;

protected:
    [[nodiscard]] bool shouldParallelize() const final
    {
        return false;
    }

    void PreRunInitialization();

    void BuildMasterProblem();

    void SolveSubproblem(PlainData::SubProblemData& subproblem_data,
                         const std::string& name,
                         const std::shared_ptr<SubproblemWorker>& worker) override;
};
