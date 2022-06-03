// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <filesystem>

#include "JsonWriter.h"
#include "MergeMPS.h"
#include "SimulationOptions.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "glog/logging.h"
#include "logger/User.h"
#include "solver_utils.h"

//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char **argv) {
  usage(argc);
  SimulationOptions options(argv[1]);
  options.print(std::cout);

  Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

  google::InitGoogleLogging(argv[0]);
  auto path_to_log = std::filesystem::path(options.OUTPUTROOT) / "merge_mpsLog";
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());
  LOG(INFO) << "starting merge_mps" << std::endl;

  Writer writer =
      build_json_writer(std::filesystem::path(options.JSON_FILE), false);
  try {
    MergeMPS merge_mps(options.get_base_options(), logger, writer);
    merge_mps.launch();
  } catch (std::exception &ex) {
    std::string error =
        "Exception raised and program stopped : " + std::string(ex.what());
    LOG(WARNING) << error << std::endl;
    logger->display_message(error);
    exit(1);
  }

  return 0;
}
