#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"

namespace po = boost::program_options;

BellmanValuesExeOptions::BellmanValuesExeOptions():
    OptionsParser("Bellman Values computation exe")
{
    AddOptions()("help,h",
                 "produce help message")("study",
                                         po::value<std::filesystem::path>(&studyPath_)->required(),
                                         "Path to archive (required)")(
      "solver",
      po::value<std::string>(&solverName_)->default_value("xpress"),
      "Solver to use (optional, default is xpress). Possible values are: xpress, cbc, clp")(
      "threads",
      po::value<int>(&nbThreads_)->default_value(1),
      "Number of threads to use (optional, default is 1)");
}
