#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"

namespace po = boost::program_options;

BellmanValuesExeOptions::BellmanValuesExeOptions():
    OptionsParser("Bellman Values computation exe")
{
    AddOptions()("help,h",
                 "produce help message")("study",
                                         po::value<std::filesystem::path>(&studyPath_)->required(),
                                         "Path to archive")(
      "solver",
      po::value<std::string>(&solverName_),
      "Solver to use")("threads", po::value<int>(&nbThreads_), "Number of threads to use");
}
