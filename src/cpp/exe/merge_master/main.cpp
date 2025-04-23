// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <filesystem>

#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/Worker.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"
#include "antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/helpers/solver_utils.h"

//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char** argv)
{
    usage(argc);
    SimulationOptions options(argv[1]);
    options.print(std::cout);

    // // This is temporary
    // std::string test_set = "simple_tree";
    // MergeMasterMPSOptions options {
    //     .INPUTROOT = "/home/bessinnic/Documents/antares-xpansion/merge_master_test/" + test_set,
    //     .OUTPUTROOT = "/home/bessinnic/Documents/antares-xpansion/merge_master_test/" + test_set + "/output",
    //     .STRUCTURE_FILE = "master_structure_minimal.json",
    //     .SOLVER_TO_USE = "CBC",
    //     .LOG_LEVEL = 1,
    // };

    std::string output_file = options.OUTPUTROOT + "/output.json";

    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("starting merge_mps");

    std::shared_ptr<Output::OutputWriter> writer = build_json_writer(std::filesystem::path(
                                                                       output_file),
                                                                     false);
    try
    {
        MergeMasterTrajectoryMPS merge_master_mps(options.get_base_options(), logger, writer);
        merge_master_mps.launch(); // We hardcode the behaviour for now, no change from previously
    }
    catch (std::exception& ex)
    {
        std::string error = "Exception raised and program stopped : " + std::string(ex.what());
        logger->display_message(error);
        exit(1);
    }

    return 0;
}
