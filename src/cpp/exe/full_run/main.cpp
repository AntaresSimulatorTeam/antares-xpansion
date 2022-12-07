#include <exception>
#include <iostream>
#include <string>

#if __has_include("BendersMpiMain.h")
#include "BendersMpiMain.h"
#include "common_mpi.h"
#endif
#include <boost/program_options.hpp>

#include "BendersSequentialMain.h"
#include "FullRunOptionsParser.h"
#include "ProblemGenerationLogger.h"
#include "RunProblemGeneration.h"
#include "StudyUpdateRunner.h"

namespace po = boost::program_options;

int main(int argc, char** argv) {
#ifdef BENDERSMPIMAIN
  mpi::environment env(argc, argv);
  mpi::communicator world;
#endif
  auto options_parser = FullRunOptionsParser();
  std::filesystem::path root;
  std::filesystem::path archive_path;
  options_parser.Parse(argc, argv);

#ifdef BENDERSMPIMAIN
  if (world.rank() == 0) {
#endif
    try {
      root = options_parser.Root();
      archive_path = options_parser.ArchivePath();

      const auto log_file_path = root / "lp" / "ProblemGenerationLog.txt";
      auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout);
      auto& loggerRef = (*logger);

      auto master_formulation = options_parser.MasterFormulation();
      auto additionalConstraintFilename_l =
          options_parser.AdditionalConstraintsFilename();
      auto zip_mps = options_parser.ZipMps();
      RunProblemGeneration(root, master_formulation,
                           additionalConstraintFilename_l, archive_path, logger,
                           log_file_path, zip_mps);

    } catch (std::exception& e) {
      std::cerr << "error: " << e.what() << std::endl;
      return 1;
    } catch (...) {
      std::cerr << "Exception of unknown type!" << std::endl;
    }
#ifdef BENDERSMPIMAIN
  }
#endif
  int argc_ = 2;
  auto options_file = options_parser.BendersOptionsFile();

#ifdef BENDERSMPIMAIN
  world.barrier();
  if (options_parser.Method() == FullRunOptionsParser::METHOD::MPI) {
    BendersMpiMain(argc_, argv, options_file, env, world);
  } else {
    if (world.rank() == 0) {
      BendersSequentialMain(argc_, argv, options_file);
    }
  }
#else
  BendersSequentialMain(argc_, argv, options_file);
#endif

#ifdef BENDERSMPIMAIN
  if (world.rank() == 0) {
#endif
    auto log_file_path = root / "lp" / "StudyUpdateLog.txt";
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout);
    auto solutionFile_l = options_parser.SolutionFile();
    ActiveLinksBuilder linksBuilder = get_link_builders(root, logger);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(root, links, solutionFile_l, logger);

#ifdef BENDERSMPIMAIN
  }
#endif
  return 0;
}
