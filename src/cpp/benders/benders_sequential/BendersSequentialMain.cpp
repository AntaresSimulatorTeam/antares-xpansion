
#include "BendersSequentialMain.h"

#include <filesystem>

#include "BendersSequential.h"
#include "LoggerFactories.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "StartUp.h"
#include "Timer.h"
#include "WriterFactories.h"
#include "glog/logging.h"

int RunSequential(char **argv, const std::filesystem::path &options_file) {
  SimulationOptions options(options_file);
  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      std::filesystem::path(options.OUTPUTROOT) / "benderssequentialLog.txt.";
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  std::ostringstream oss_l = start_message(options, "Sequential");
  LOG(INFO) << oss_l.str() << std::endl;

  const auto &loggerFileName =
      std::filesystem::path(options.OUTPUTROOT) / "reportbenderssequential.txt";

  auto logger_factory = FileAndStdoutLoggerFactory(loggerFileName);

  Logger logger = logger_factory.get_logger();
  Writer writer = build_json_writer(options.JSON_FILE, options.RESUME);
  if (Benders::StartUp startup;
      startup.StudyAlreadyAchievedCriterion(options, writer, logger))
    return 0;
  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);

  BendersSequential benders(benders_options, logger, writer);
  benders.set_log_file(loggerFileName);
  benders.launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(benders.execution_time());

  return 0;
}
int BendersSequentialMain(int argc, char **argv) {
  usage(argc);
  return RunSequential(argv, argv[1]);
}
int BendersSequentialMain(int argc, char **argv,
                          const std::filesystem::path &options_file) {
  usage(argc);
  return RunSequential(argv, options_file);
}