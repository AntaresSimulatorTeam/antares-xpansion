//
// Created by marechaljas on 02/08/22.
//

#include "antares-xpansion/benders/benders_core/StartUp.h"

namespace Benders
{

bool StartUp::StudyAlreadyAchievedCriterion(const SimulationOptions& options,
                                            Output::OutputWriter* writer,
                                            ILogger* logger) const
{
    if (options.RESUME != ResumeMode::RESUME)
    {
        return false;
    }
    if (writer->solution_status() == Output::OPTIMAL_C)
    {
        std::stringstream str;
        str << "Study is already optimal " << std::endl
            << "Optimization results available in : " << options.JSON_FILE;
        logger->display_message(str.str());
        return true;
    }
    return false;
}

} // namespace Benders
