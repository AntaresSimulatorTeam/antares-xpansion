#include <exception>
#include <iostream>
#include <string>

#if __has_include("BendersMpiMain.h")
#include "BendersMpiMain.h"
#endif
#include <boost/program_options.hpp>

#include "BendersSequentialMain.h"
#include "FullRunOptionsParser.h"
#include "RunProblemGeneration.h"
#include "StudyUpdateRunner.h"

namespace po = boost::program_options;
// METHOD OptionsReader(int argc, char** argv) {
//   po::options_description desc("Allowed options");

//   desc.add_options()("help,h", "produce help message")(
//       "output,o", po::value<std::filesystem::path>(&root)->required(),
//       "antares-xpansion study output")(
//       "formulation,f",
//       po::value<std::string>(&master_formulation)->default_value("relaxed"),
//       "master formulation (relaxed or integer)")(
//       "exclusion-files,e",
//       po::value<std::string>(&additionalConstraintFilename_l),
//       "path to exclusion files");

//   po::variables_map opts;
//   po::store(po::parse_command_line(argc, argv, desc), opts);

// if (opts.count("help")) {
//   std::cout << desc << std::endl;
//   return 0;
// }

// po::notify(opts);
// }

int main(int argc, char** argv) {
  auto options_parser = FullRunOptionsParser();
  std::filesystem::path root;
  try {
    options_parser.Parse(argc, argv);
    root = options_parser.Root();
    auto master_formulation = options_parser.MasterFormulation();
    auto additionalConstraintFilename_l =
        options_parser.additional_constraintFilename_l();
    RunProblemGeneration(root, master_formulation,
                         additionalConstraintFilename_l);

  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }
  int argc_ = 2;
  auto options_file = options_parser.BendersOptionsFile();

#ifdef BENDERSMPIMAIN
  if (options_parser.Method() == FullRunOptionsParser::METHOD::MPI) {
    BendersMpiMain(argc_, argv, options_file);
  } else {
    BendersSequentialMain(argc_, argv, options_file);
  }
#endif

  auto solutionFile_l = options_parser.SolutionFile();
  ActiveLinksBuilder linksBuilder = get_link_builders(root);

  const std::vector<ActiveLink> links = linksBuilder.getLinks();

  updateStudy(root, links, solutionFile_l);
  return 0;
}
