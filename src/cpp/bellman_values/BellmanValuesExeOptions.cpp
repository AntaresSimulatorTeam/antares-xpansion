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
      "Solver to use (optional, default is xpress). Possible values are: xpress, coin")(
      "threads",
      po::value<int>(&nbThreads_)->default_value(1),
      "Number of threads to use (optional, default is 1)")(
      "start-week",
      po::value<int>(&startWeek_)->default_value(1),
      "Start week (optional, default is 1)")("end-week",
                                             po::value<int>(&endWeek_)->default_value(52),
                                             "End week (optional, default is 52)")(
      "nb-levels",
      po::value<int>(&nbLevels_)->default_value(10),
      "Number of levels (optional, default is 10)")(
      "antares-format",
      po::value<bool>(&antaresFormat_)->default_value(false),
      "Output in Antares format (optional, default is false)")(
      "write-pb-files",
      po::value<bool>(&writePbFiles_)->default_value(false),
      "Write MPS or SVF files to disk (optional, default is false)");
}
