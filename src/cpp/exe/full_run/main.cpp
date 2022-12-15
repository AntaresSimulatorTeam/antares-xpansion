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
#ifdef __BENDERSMPI__
  mpi::environment env(argc, argv);
  mpi::communicator world;
#endif
  auto options_parser = FullRunOptionsParser();
  std::filesystem::path root;
  std::filesystem::path archive_path;
  options_parser.Parse(argc, argv);

#ifdef __BENDERSMPI__
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
#ifdef __BENDERSMPI__
  }
#endif
  int argc_ = 2;
  const auto options_file = options_parser.BendersOptionsFile();
  const auto benders_method = options_parser.Method();

#ifdef __BENDERSMPI__
  world.barrier();
  if (benders_method == BENDERSMETHOD::MPI) {
    auto benders_factory =
        BendersMainFactory(argc_, argv, options_file, env, world);
    benders_factory.Run();
  } else {
    if (world.rank() == 0) {
      std::cout << "argv[0]: " << argv[0] << "\n";
      std::cout << "argv[1]: " << argv[1] << "\n";
      std::cout << "argv[2]: " << argv[2] << "\n";
      auto benders_factory =
          BendersMainFactory(argc_, argv, options_file, benders_method);
      benders_factory.Run();
    }
  }
#else
  auto benders_factory =
      BendersMainFactory(argc_, argv, options_file, benders_method);
  benders_factory.Run();
#endif

#ifdef __BENDERSMPI__
  if (world.rank() == 0) {
#endif
    auto log_file_path = root / "lp" / "StudyUpdateLog.txt";
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout);
    auto solutionFile_l = options_parser.SolutionFile();
    ActiveLinksBuilder linksBuilder = get_link_builders(root, logger);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(root, links, solutionFile_l, logger);

#ifdef __BENDERSMPI__
  }
#endif
  return 0;
}
