// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <filesystem>

#include "BendersSequential.h"
#include "JsonWriter.h"
#include "LoggerFactories.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "Timer.h"
#include "WriterFactories.h"
#include "glog/logging.h"
int main(int argc, char **argv) {
  usage(argc);
  std::unique_ptr<SimulationOptions> options;
  if (argc == 2) {
    options = std::make_unique<SimulationOptions>(argv[1]);
  } else {
    options = std::make_unique<SimulationOptions>(argv[1], argv[2]);
  }
  //options->print(std::cout);
  BendersBaseOptions benders_options(options->get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      std::filesystem::path(options->OUTPUTROOT) / "benderssequentialLog.txt.";
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  std::ostringstream oss_l = start_message(*options, "Sequential");
  LOG(INFO) << oss_l.str() << std::endl;

  const auto &loggerFileName =
      std::filesystem::path(options->OUTPUTROOT) / "reportbenderssequential.txt";

  auto logger_factory = FileAndStdoutLoggerFactory(loggerFileName);

  Logger logger = logger_factory.get_logger();
  Writer writer = build_json_writer(options->JSON_FILE, options->RESUME);
  if (options->RESUME && writer->solution_status() == Output::STATUS_OPTIMAL_C) {
    std::stringstream str;
    str << "Study is already optimal " << std::endl
        << "Optimization results available in : " << options->JSON_FILE;
    logger->display_message(str.str());
    return 0;
  }
  writer->write_log_level(options->LOG_LEVEL);
  writer->write_master_name(options->MASTER_NAME);
  writer->write_solver_name(options->SOLVER_NAME);

  BendersSequential benders(benders_options, logger, writer);
  benders.set_log_file(loggerFileName);
  benders.launch();
  std::stringstream str;
  str << "Optimization results available in : " << options->JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(benders.execution_time());

  return 0;
}
