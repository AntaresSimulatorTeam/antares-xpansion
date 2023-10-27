#include "ProblemGenerationExeOptions.h"
namespace po = boost::program_options;

ProblemGenerationExeOptions::ProblemGenerationExeOptions()
    : OptionsParser("Problem Generation exe") {
  AddOptions()("help,h", "produce help message")(
      "output,o",
      po::value<std::filesystem::path>(&xpansion_output_dir_)->required(),
      "antares-xpansion study output")(
      "archive,a", po::value<std::filesystem::path>(&archive_path_)->required(),
      "antares-xpansion study zip")(
      "formulation,f",
      po::value<std::string>(&master_formulation_)->default_value("relaxed"),
      "master formulation (relaxed or integer)")(
      "exclusion-files,e",
      po::value<std::string>(&additional_constraintFilename_l_),
      "path to exclusion files")(
      "weights-file,w",
      po::value<std::filesystem::path>(&weights_file_)->default_value(""),
      "user weights file")("unnamed-problems,n",
                           po::bool_switch(&unnamed_problems_),
                           "use this option if unnamed problems are provided");
}
void ProblemGenerationExeOptions::Parse(unsigned int argc,
                                        const char *const *argv) {
  OptionsParser::Parse(argc, argv);
}
