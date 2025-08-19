#include "antares-xpansion/full_run/FullRunOptionsParser.h"

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
namespace po = boost::program_options;

FullRunOptionsParser::FullRunOptionsParser():
    ProblemGenerationExeOptions()
{
    AddOptions()("benders_options,b",
                 po::value<std::filesystem::path>(&benders_options_file_)->required(),
                 "benders options file")(
      "solution,s",
      po::value<std::filesystem::path>(&solutionFile_)->required(),
      "path to json solution file")("solver",
                                    po::value<std::string>(&solver_)->default_value("benders"),
                                    "solver (benders, outer_loop, "); // Add mergeMps?
    AddOptions()("presolve",
                 po::value<bool>(&presolve_)->default_value(true),
                 "use presolve (default: true)");
}

void FullRunOptionsParser::Parse(unsigned int argc, const char* const* argv)
{
    ProblemGenerationExeOptions::Parse(argc, argv);
}

std::string FullRunOptionsParser::Solver() const
{
    return solver_;
}

bool FullRunOptionsParser::presolve() const
{
    return presolve_;
}
