// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "glog/logging.h"

#include "MergeMPS.h"
#include "Worker.h"
#include "SimulationOptions.h"
#include "JsonWriter.h"

#include "solver_utils.h"
#include "logger/User.h"
#include "helpers/Path.h"
#include "WriterFactories.h"

//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char **argv)
{
    usage(argc);
    SimulationOptions options(argv[1]);
    options.print(std::cout);

    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    google::InitGoogleLogging(argv[0]);
    auto path_to_log = (Path(options.OUTPUTROOT) / "merge_mpsLog").get_str();
    google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());
    LOG(INFO) << "starting merge_mps" << std::endl;

    Writer writer = build_json_writer(options.JSON_FILE);
    try
    {
        MergeMPS merge_mps(options.get_base_options(), logger, writer);
        merge_mps.launch();
    }
    catch (std::exception &ex)
    {
        std::string error = "Exception raised and program stopped : " + std::string(ex.what());
        LOG(WARNING) << error << std::endl;
        logger->display_message(error);
        exit(1);
    }

    return 0;
}
