#include <boost/program_options.hpp>
#include <exception>
#include <iostream>

#include "FullRunOptionsParser.h"
#include "ProblemGeneration.h"
#include "ProblemGenerationLogger.h"
#include "StudyUpdateRunner.h"
#include "common_mpi.h"
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
  const auto benders_method = options_parser.Method();

  auto benders_factory =
      BendersMainFactory(argc_, argv, benders_method, options_file, env, world);
  benders_factory.Run();

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
