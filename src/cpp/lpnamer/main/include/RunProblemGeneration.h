#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H
#include <filesystem>
#include <string>

#include "ProblemGenerationExeOptions.h"
#include "ProblemGenerationLogger.h"

void RunProblemGeneration(
    const std::filesystem::path& xpansion_output_dir,
    const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    const std::filesystem::path& archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path, bool zip_mps,
    const std::filesystem::path& weights_file);
void ProcessWeights(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& weights_file,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
void ExtractUtilsFiles(
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

#endif  // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_RUNPROBLEMGENERATION_H
