#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H
#include <filesystem>
#include <string>

#include "ProblemGenerationExeOptions.h"
#include "ProblemGenerationLogger.h"

void RunProblemGeneration(
    const std::filesystem::path& root, const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    std::filesystem::path archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path, bool zip_mps);
#endif  // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H