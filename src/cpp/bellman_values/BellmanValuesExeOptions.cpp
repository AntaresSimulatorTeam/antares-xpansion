#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"

namespace po = boost::program_options;

BellmanValuesExeOptions::BellmanValuesExeOptions():
    OptionsParser("Bellman Values computation exe")
{
    AddOptions()("help,h", "produce help message")(
      "study",
      po::value<std::filesystem::path>(&studyPath_)->required(),
      "Path to archive (required)")("threads",
                                    po::value<int>(&nbThreads_)->default_value(1),
                                    "Number of threads to use (optional, default is 1)");
}
