#include <exception>
#include <iostream>
#include <string>

#if __has_include("common_mpi.h")
#include "common_mpi.h"
#endif
#include <boost/program_options.hpp>

#include "FullRunOptionsParser.h"
#include "ProblemGenerationLogger.h"
#include "RunProblemGeneration.h"
#include "StudyUpdateRunner.h"
namespace po = boost::program_options;

int main(int argc, char** argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;
  auto options_parser = FullRunOptionsParser();
  std::filesystem::path xpansion_output_dir;
  std::filesystem::path archive_path;
  options_parser.Parse(argc, argv);

  if (world.rank() == 0) {
    try {
      xpansion_output_dir = options_parser.XpansionOutputDir();
      archive_path = options_parser.ArchivePath();

      const auto log_file_path =
          xpansion_output_dir / "lp" / "ProblemGenerationLog.txt";
      auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout);
      auto& loggerRef = (*logger);

      auto master_formulation = options_parser.MasterFormulation();
      auto additionalConstraintFilename_l =
          options_parser.AdditionalConstraintsFilename();
      auto zip_mps = options_parser.ZipMps();
      auto weights_file = options_parser.WeightsFile();

      RunProblemGeneration(xpansion_output_dir, master_formulation,
                           additionalConstraintFilename_l, archive_path, logger,
                           log_file_path, zip_mps, weights_file);

    } catch (std::exception& e) {
      std::cerr << "error: " << e.what() << std::endl;
      return 1;
    } catch (...) {
      std::cerr << "Exception of unknown type!" << std::endl;
    }
  }
  int argc_ = 2;
  const auto options_file = options_parser.BendersOptionsFile();
  const auto benders_method = options_parser.Method();

  world.barrier();
  auto benders_factory =
      BendersMainFactory(argc_, argv, benders_method, options_file, env, world);
  benders_factory.Run();

  if (world.rank() == 0) {
    auto log_file_path = xpansion_output_dir / "lp" / "StudyUpdateLog.txt";
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout);
    auto solutionFile_l = options_parser.SolutionFile();
    ActiveLinksBuilder linksBuilder =
        get_link_builders(xpansion_output_dir, logger);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(xpansion_output_dir, links, solutionFile_l, logger);
  }
  return 0;
}
