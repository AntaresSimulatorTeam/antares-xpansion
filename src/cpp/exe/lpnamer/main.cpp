
#include <iostream>

#include "ProblemGenerationExeOptions.h"
#include "ProblemGenerationLogger.h"
#include "RunProblemGeneration.h"

static const std::string LP_DIRNAME = "lp";

void CreateDirectories(const std::filesystem::path& output_path) {
  if (!std::filesystem::exists(output_path)) {
    std::filesystem::create_directories(output_path);
  }
  auto lp_path = output_path / LP_DIRNAME;
  if (!std::filesystem::exists(lp_path)) {
    std::filesystem::create_directories(lp_path);
  }
}

int main(int argc, char** argv) {
  try {
    auto options_parser = ProblemGenerationExeOptions();
    options_parser.Parse(argc, argv);

    auto xpansion_output_dir = options_parser.XpansionOutputDir();
    auto archive_path = options_parser.ArchivePath();
    using namespace ProblemGenerationLog;
    const auto log_file_path =
        xpansion_output_dir / "lp" / "ProblemGenerationLog.txt";

    CreateDirectories(xpansion_output_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                    "Problem Generation");

    auto master_formulation = options_parser.MasterFormulation();
    auto additionalConstraintFilename_l =
        options_parser.AdditionalConstraintsFilename();
    auto weights_file = options_parser.WeightsFile();
    auto unnamed_problems = options_parser.UnnamedProblems();
    RunProblemGeneration(xpansion_output_dir, master_formulation,
                         additionalConstraintFilename_l, archive_path, logger,
                         log_file_path, weights_file, unnamed_problems);

    return 0;
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }

  return 0;
}