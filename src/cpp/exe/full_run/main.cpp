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
  options_parser.parse(argc, argv);
  try {
    auto root = options_parser.root();
    auto master_formulation = options_parser.master_formulation();
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
  std::vector<char> cstr(options_file.c_str(),
                         options_file.c_str() + options_file.size() + 1);
  std::vector<char*> argv_ = {"", cstr.data()};

#ifdef BENDERSMPIMAIN
  if (options_parser.Method() == FullRunOptionsParser::METHOD::MPI) {
    BendersMpiMain(argc_, argv_.data());
  } else {
    BendersSequentialMain(argc_, argv_.data());
  }
#endif
  return 0;
}
