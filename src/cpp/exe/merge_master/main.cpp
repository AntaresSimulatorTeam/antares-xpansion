#include <filesystem>

#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/benders/merge_mps/MergeMPS.h"
#include "antares-xpansion/benders/merge_mps/StandardLp.h"

int main(int argc, char** argv)
{
    usage(argc);
    SimulationOptions options(argv[1]);
    options.print(std::cout);

    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("starting merge_master_mps");

    std::shared_ptr<Output::OutputWriter> writer = build_json_writer(std::filesystem::path(
                                                                       options.JSON_FILE),
                                                                     false);
    try
    {
        logger->display_message("Given tree path is : " + std::string(argv[2]));
        MergePathwayMPS merge_mps(options.get_base_options(), logger, writer, argv[2]);
        merge_mps.launch();
    }
    catch (std::exception& ex)
    {
        std::string error = "Exception raised and program stopped : " + std::string(ex.what());
        logger->display_message(error);
        exit(1);
    }

    return 0;
}
