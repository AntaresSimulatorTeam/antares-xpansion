#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <filesystem>
#include <memory>
#include <string>

#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

class SkeletonSolverLoader
{
public:
    explicit SkeletonSolverLoader(Logger& logger);

    std::shared_ptr<SolverAbstract> Load(const std::filesystem::path& mps_path,
                                         const std::string& solver_name,
                                         const SolverLogManager& solver_log_manager,
                                         int log_level,
                                         ProblemsFormat format);

private:
    Logger logger_;
};
