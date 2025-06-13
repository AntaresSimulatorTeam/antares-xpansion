#pragma once

// #include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"
#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"

class MultipleProblemGenerationExeOptions: public ProblemGenerationExeOptions
{
private:
    // Path to which we want to write the nodal_lp_info obtained after problem generation.
    std::filesystem::path nodal_lp_info_path_;

public:
    MultipleProblemGenerationExeOptions();

    ~MultipleProblemGenerationExeOptions() override = default;

    void Parse(unsigned int argc, const char* const* argv) override;

    void checkMandatoryOptions(const std::string& log_location) const;

    std::filesystem::path NodalLpInfoPath() const
    {
        return nodal_lp_info_path_;
    }
};

class MultipleProblemGeneration
{
public:
    explicit MultipleProblemGeneration(MultipleProblemGenerationExeOptions& options):
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
        Parse a list of paths provided by the user in the form of a text file with two columns :
        node_name1 path/to/archive
        node_name2 or/path/to/output
        node_name3 or/path/to/study
        ...
        Wether the path corresponds to a study, Antares output or archive does not matter for this
        function
    */
    void load_input_paths();

    /*
        Parse a list of weight files provided by the user in the form of a text file with two
       columns : node_name1 path/to/weight/file node_name3 path/to/weight/file
        ...
        If a node is absent from this file, it will be assumed to have uniform weights.
    */
    void load_input_weight_files();
    /*
        Writes the lp_folder paths to a Json file
    */
    void write_lp_paths() const;

private:
    typedef std::map<std::string, std::filesystem::path> NodeToPathMap;
    // Inputs
    const MultipleProblemGenerationExeOptions& options_;
    // ProblemGenerationLog::ProblemGenerationLogger logger_;   //TODO Add Logger
    NodeToPathMap node_to_input_path_;
    NodeToPathMap node_to_weight_file_;
    // Outputs
    NodesToLpDataLocationMap node_to_lp_info_;
};
