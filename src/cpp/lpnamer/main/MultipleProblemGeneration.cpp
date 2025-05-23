#include "antares-xpansion/lpnamer/main/MultipleProblemGeneration.h"

#include <json/writer.h>

namespace
{
void check_format(const std::vector<std::string>& split)
{
    const std::string file_format_error{"Paths file should have two columns separated by ' ' : \n "
                                        "node_study_name1 path/to/archive \n"
                                        "node_study_name2 or/path/to/output \n"
                                        "node_study_name3 or/path/to/study"};
    if (split.size() != 2)
    {
        std::cerr << file_format_error << std::endl;
        std::exit(1);
    }
}
} // namespace

void MultipleProblemGeneration::load_input_paths()
{
    const std::string OUTPUT = "output";

    const auto path = options_.getRelevantPath();

    std::ifstream f(path);
    std::string line;

    // First line should be : 'output path/to/output/file'
    std::getline(f, line);
    auto split = StringManip::split(line, " ");
    check_format(split);
    if (split[0] != OUTPUT)
    {
        std::cerr << "First line should be : 'output path/to/output/file'";
        std::exit(1);
    }
    output_filepath_ = split[1];

    // Nodal studies input paths
    while (std::getline(f, line))
    {
        // Quick and dirty, perhaps not very robust ?
        auto split = StringManip::split(line, " ");
        check_format(split);
        node_to_input_path_[split[0]] = split[1];
    }
}

void MultipleProblemGeneration::run_generation()
{
    for (const auto& [node, input_path]: node_to_input_path_)
    {
        auto individual_options = ProblemGenerationExeOptions(options_);
        individual_options.setRelevantPath(input_path);
        auto pbg = ProblemGeneration(individual_options);
        std::filesystem::path output_folder = pbg.updateProblems();

        node_to_lp_folder_[node] = output_folder / "lp/";
        std::cout << "Successfully generated lp_folder and files for problem : " << node
                  << std::endl;
    }
}

void MultipleProblemGeneration::write_lp_paths()
{
    // Could be linked with the same constant defined in merge_master_mps/MasterStructureKeys.h
    const char KEY_LP_FOLDER[] = "lp_folder";

    Json::Value output;
    for (const auto& [name, lp_folder]: node_to_lp_folder_)
    {
        output[name][KEY_LP_FOLDER] = lp_folder.string();
    }

    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    std::ofstream outputFileStream(output_filepath_);
    writer->write(output, &outputFileStream);
    std::cout << "Successfully written lp_paths to file : " << output_filepath_ << std::endl;
    outputFileStream.close();
}
