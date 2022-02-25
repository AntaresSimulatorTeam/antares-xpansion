// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include "BendersSequential.h"
#include "JsonWriter.h"
#include "LoggerFactories.h"
#include "OutputWriter.h"
#include "SimulationOptions.h"
#include "Timer.h"
#include "WriterFactories.h"
#include "glog/logging.h"
#include "helpers/Path.h"

int main(int argc, char **argv) {
  // options.print(std::cout);
  usage(argc);
  SimulationOptions options(argv[1]);
  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      (Path(options.OUTPUTROOT) / "benderssequentialLog.txt.").get_str();
  google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());

  std::ostringstream oss_l = start_message(options, "Sequential");
  LOG(INFO) << oss_l.str() << std::endl;

  const std::string &loggerFileName =
      (Path(options.OUTPUTROOT) / "reportbenderssequential.txt").get_str();

  auto logger_factory = FileAndStdoutLoggerFactory(loggerFileName);

  Logger logger = logger_factory.get_logger();
  Writer writer = build_json_writer(options.JSON_FILE);
  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);
  Timer timer;

  BendersSequential benders(benders_options, logger, writer);
  benders.set_log_file(loggerFileName);
  benders.launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(timer.elapsed());

  return 0;
}
