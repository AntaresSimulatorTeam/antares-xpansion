#include "FullRunOptionsParser.h"
namespace po = boost::program_options;

FullRunOptionsParser::FullRunOptionsParser() : ProblemGenerationExeOptions() {
  AddOptions()("method,m", po::value<std::string>(&method_str_)->required(),
               "benders method")(
      "benders_options,b",
      po::value<std::filesystem::path>(&benders_options_file_)->required(),
      "benders options file")(
      "solution,s",
      po::value<std::filesystem::path>(&solutionFile_)->required(),
      "path to json solution file");
}
void FullRunOptionsParser::Parse(unsigned int argc, const char* const* argv) {
  OptionsParser::Parse(argc, argv);
  SetMethod();
}
void FullRunOptionsParser::SetMethod() {
  if (method_str_ == "sequential") {
    method_ = METHOD::SEQUENTIAL;
  } else if (method_str_ == "mpibenders") {
    method_ = METHOD::MPI;
  } else if (method_str_ == "mergeMPS") {
    method_ = METHOD::MERGEMPS;
  } else {
    throw FullRunOptionsParser::FullRunOptionInvalidMethod(method_str_);
  }
}