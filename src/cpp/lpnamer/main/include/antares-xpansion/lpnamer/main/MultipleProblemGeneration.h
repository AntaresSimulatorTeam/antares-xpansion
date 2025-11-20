#pragma once

#include "ConfigurationManager.h"
#include "antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"

class MultipleProblemGenerationExeOptions: public ProblemGenerationExeOptions
{
private:
    // Path to which we want to write the nodal_lp_info obtained after problem generation.
    std::filesystem::path nodal_lp_info_path_;
    // Input root, used to write the paths as relative to the input root
    std::filesystem::path input_root_;

public:
    MultipleProblemGenerationExeOptions();

    ~MultipleProblemGenerationExeOptions() override = default;

    void Parse(unsigned int argc, const char* const* argv) override;

    void checkMandatoryOptions(const std::string& log_location) const;

    std::filesystem::path NodalLpInfoPath() const
    {
        return nodal_lp_info_path_;
    }

    // Path to the folder containing the studies we work on.
    std::filesystem::path InputRootPath() const
    {
        return input_root_;
    }
};

class MultipleProblemGeneration
{
public:
    explicit MultipleProblemGeneration(MultipleProblemGenerationExeOptions& options);

    virtual ~MultipleProblemGeneration() = default;

private:
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
       columns :
       node_name1 path/to/weight/file
       node_name2 path/to/weight/file
        ...
        If a node is absent from this file, it will be assumed to have no custom weights.
    */
    void load_input_weight_files();
    /*
        Parse a list of additional constraints files provided by the user in the form of a text file
       with two columns : *
       node_name1 path/to/constraints/file
       node_name2 path/to/constraints/file
        ...
        If a node is absent from this file, it will be assumed to have additional constraints.
    */
    void load_additional_constraints_files();

public:
    /*
        Load all the necessary data
    */
    void load_data();

    /*
        Runs the problem generation for every node given in the input file
    */
    void run_generation();
    /*
        Writes the lp_folder paths to a Json file
    */
    void write_lp_paths() const;

private:
    using NodeToPathMap = std::map<std::string, std::filesystem::path>;
    // Inputs
    const MultipleProblemGenerationExeOptions& options_;
    NodeToPathMap node_to_input_path_;
    NodeToPathMap node_to_weight_file_;
    NodeToPathMap node_to_additional_constraints_file_;
    // Outputs
    NodesToLpDataLocationMap node_to_lp_info_;
    // TODO // Logger
    // std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger_;
    // ConfigurationManager configuration_manager_;
    // ConfigurationManager::ConfigDirectories directories_;
};
