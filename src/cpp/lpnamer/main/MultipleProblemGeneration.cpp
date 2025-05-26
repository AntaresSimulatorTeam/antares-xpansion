#include "antares-xpansion/lpnamer/main/MultipleProblemGeneration.h"

namespace po = boost::program_options;

MultipleProblemGenerationExeOptions::MultipleProblemGenerationExeOptions():
    ProblemGenerationExeOptions()
{
    AddOptions()("nodal-file",
                 po::value<std::filesystem::path>(&nodal_lp_info_path_),
                 "nodal_lp_info output filepath");
}

void MultipleProblemGenerationExeOptions::Parse(unsigned int argc, const char* const* argv)
{
    OptionsParser::Parse(argc, argv);
    auto log_location = LOGLOCATION;
    checkMandatoryOptions(log_location);
}

void MultipleProblemGenerationExeOptions::checkMandatoryOptions(
  const std::string& log_location) const
{
    ProblemGenerationExeOptions::checkMandatoryOptions(log_location);

    if (nodal_lp_info_path_.empty())
    {
        auto msg = "--nodal-file must be given.";
        throw ProblemGenerationOptions::ConflictingParameters(msg, log_location);
    }
}

namespace
{
void check_format(const std::vector<std::string>& split, const std::string& error_message)
{
    if (split.size() != 2)
    {
        std::cerr << error_message << std::endl;
        std::exit(1);
    }
}
} // namespace

void MultipleProblemGeneration::load_input_paths()
{
    const std::string file_format_error{"paths file should have two columns separated by ' ' : \n "
                                        "node_study_name1 path/to/archive \n"
                                        "node_study_name2 or/path/to/output \n"
                                        "node_study_name3 or/path/to/study"};
    const auto path = options_.getRelevantPath();
    std::ifstream f(path);
    std::string line;

    // Nodal studies input paths
    while (std::getline(f, line))
    {
        // Quick and dirty, perhaps not very robust / fast ?
        auto split = StringManip::split(line, " ");
        check_format(split, file_format_error);
        node_to_input_path_[split[0]] = split[1];
    }
}

void MultipleProblemGeneration::load_input_weight_files()
{
    const std::string file_format_error{
      "Weights info file should have two columns separated by ' ' : \n "
      "node_study_name1 path/to/weight/file \n"
      "node_study_name2 path/to/weight/file \n"
      "... \n"
      "If a node is absent, it will be assumed to have uniform weights"};
    const auto path = options_.WeightsFile();
    if (path.empty())
    {
        return;
    }
    std::ifstream f(path);
    std::string line;

    // Nodal weight files paths
    while (std::getline(f, line))
    {
        auto split = StringManip::split(line, " ");
        check_format(split, file_format_error);
        node_to_weight_file_[split[0]] = split[1];
    }
}

void MultipleProblemGeneration::run_generation()
{
    for (const auto& [node, input_path]: node_to_input_path_)
    {
        auto individual_options = ProblemGenerationExeOptions(options_);
        individual_options.setRelevantPath(input_path);
        // Weights file
        if (node_to_weight_file_.contains(node))
        {
            individual_options.setWeightsFilePath(node_to_weight_file_.at(node));
        }
        else
        {
            individual_options.setWeightsFilePath(std::filesystem::path());
        }
        auto pbg = ProblemGeneration(individual_options);
        std::filesystem::path output_folder = pbg.updateProblems();

        auto lp_folder = output_folder / "lp/";
        auto node_lp_location = NodeLpDataLocation(lp_folder);
        // The weight file outputted by ProblemGeneration
        // has the same name as the user input weight file
        node_lp_location.weights = node_to_weight_file_.at(node).filename();

        node_to_lp_info_.emplace(std::make_pair(node, node_lp_location));
        std::cout << "Successfully generated lp_folder and files for problem : " << node
                  << std::endl;
    }
}

void MultipleProblemGeneration::write_lp_paths()
{
    LpDataLocationManager::write_nodal_lp_location_file(node_to_lp_info_,
                                                        options_.NodalLpInfoPath());
}
