#include "FullRunOptionsParser.h"

#include "LogUtils.h"
namespace po = boost::program_options;

FullRunOptionsParser::FullRunOptionsParser() : ProblemGenerationExeOptions() {
  AddOptions()(
      "benders_options,b",
      po::value<std::filesystem::path>(&benders_options_file_)->required(),
      "benders options file")(
      "solution,s",
      po::value<std::filesystem::path>(&solutionFile_)->required(),
      "path to json solution file");
}
void FullRunOptionsParser::Parse(unsigned int argc, const char* const* argv) {
  ProblemGenerationExeOptions::Parse(argc, argv);
}