#include "FullRunOptionsParser.h"
namespace po = boost::program_options;

FullRunOptionsParser::FullRunOptionsParser() : ProblemGenerationExeOptions() {
  AddOptions()("method,m", po::value<std::string>(&method_)->required(),
               "benders method")(
      "benders_options,b",
      po::value<std::filesystem::path>(&benders_options_file_)->required(),
      "benders options file");
}

FullRunOptionsParser::METHOD FullRunOptionsParser::Method() const {
  if (method_ == "sequential") {
    return METHOD::SEQUANTIAL;
  } else if (method_ == "mpibenders") {
    return METHOD::MPI;
  }

  // else if(method_ == "mergeMPS")
}