#pragma once

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"

class MultipleProblemGeneration
{
public:
    explicit MultipleProblemGeneration(ProblemGenerationExeOptions& options):
        options_(options)
    {
        // TODO Add logger like in problem generation
        // logger_ = ProblemGenerationLog::BuildLogger(log_file_path,
        //                                             std::cout,
        //                                             "Multi-Problem Generation"s);
    }

    virtual ~MultipleProblemGeneration() = default;
    /*
        Runs the problem generation for every node given in the input file
    */
    void run_generation();
    /*
        Parse a list of paths provided by the user in the form of a text file with two columns
        output path/to/output/filename
        node_name1 path/to/archive
        node_name2 or/path/to/output
        node_name3 or/path/to/study
        First row contains must the path of the file to which we write where to find each LpFolder
        Wether the path corresponds to a study, Antares output or archive does not matter for this
        function
    */
    void load_input_paths();
    /*
        Writes the lp_folder paths to a Json file
    */
    void write_lp_paths();

private:
    typedef std::map<std::string, std::filesystem::path> NodeToPathMap;
    NodeToPathMap node_to_input_path_;
    NodeToPathMap node_to_lp_folder_;
    std::filesystem::path output_filepath_;
    const ProblemGenerationExeOptions& options_;
    // ProblemGenerationLog::ProblemGenerationLogger logger_;
};
