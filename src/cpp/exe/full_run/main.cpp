#include <boost/program_options.hpp>
#include <exception>
#include <iostream>

#include "antares-xpansion/full_run/FullRunOptionsParser.h"
#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/study-updater/StudyUpdateRunner.h"
#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
namespace po = boost::program_options;

int main(int argc, char** argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;
  auto options_parser = FullRunOptionsParser();
  std::filesystem::path xpansion_output_dir;
  options_parser.Parse(argc, argv);
  if (world.rank() == 0) {
    try {
      ProblemGeneration pbg(options_parser);
      xpansion_output_dir = pbg.updateProblems();
      std::filesystem::copy_file(
          xpansion_output_dir / "area.txt",
          xpansion_output_dir / "lp" / "area.txt",
          std::filesystem::copy_options::overwrite_existing);

    } catch (std::exception& e) {
      std::cerr << "error: " << e.what() << std::endl;
      return 1;
    } catch (...) {
      std::cerr << "Exception of unknown type!" << std::endl;
    }
  }
  world.barrier();
  int argc_ = 2;
  const auto options_file = options_parser.BendersOptionsFile();

  auto solver = options_parser.Solver();
  if (solver == "benders") {
    auto benders_factory = BendersMainFactory(argc_, argv, options_file, env,
                                              world, SOLVER::BENDERS);
    benders_factory.Run();
  }
  if (solver == "adequacy_criterion") {
    auto benders_factory = BendersMainFactory(argc_, argv, options_file, env,
                                              world, SOLVER::OUTER_LOOP);
    benders_factory.Run();
  } else {
    // TODO merge mps?
  }
  if (world.rank() == 0) {
    auto log_file_path = xpansion_output_dir / "lp" / "StudyUpdateLog.txt";
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                    "Full Run - Study Update");
    auto solutionFile_l = options_parser.SolutionFile();
    ActiveLinksBuilder linksBuilder =
        get_link_builders(xpansion_output_dir, logger);

    const std::vector<ActiveLink> links = linksBuilder.getLinks();

    updateStudy(xpansion_output_dir, links, solutionFile_l, logger);
  }
  return 0;
}
