#include <exception>
#include <iostream>
#include <string>

#ifdef BENDERSMPIMAIN
#include "BendersMpiMain.h"
#endif
#include <boost/program_options.hpp>

#include "BendersSequentialMain.h"
#include "ProblemGenerationMain.h"

namespace po = boost::program_options;
enum class METHOD { SEQUANTIAL, MPI };
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
  auto ret = ProblemGenerationMain(argc, argv);
  if (ret != 0) {
    std::string msg =
        "Error Problem Generation returned: " + std::to_string(ret);
    throw std::runtime_error(msg);
  }
#ifdef BENDERSMPIMAIN
  ret = BendersMpiMain(argc, argv);
#endif
  return 0;
}
