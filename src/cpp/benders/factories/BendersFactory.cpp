
#include "BendersFactory.h"

#include <filesystem>

#include "LoggerFactories.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

int RunBendersByBatch(char** argv, const std::filesystem::path& options_file,
                      mpi::environment& env, mpi::communicator& world) {
  SimulationOptions options(options_file);
  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      std::filesystem::path(options.OUTPUTROOT) / "benderssequentialLog.txt.";
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  std::ostringstream oss_l = start_message(options, "Sequential");
  LOG(INFO) << oss_l.str() << std::endl;

  const auto& loggerFileName =
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

  auto benders = std::make_shared<BendersByBatch>(benders_options, logger,
                                                  writer, env, world);
  benders->set_log_file(loggerFileName);
  benders->launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(benders->execution_time());

  return 0;
}

int RunBenders(char** argv, const std::filesystem::path& options_file,
               mpi::environment& env, mpi::communicator& world) {
  // Read options, needed to have options.OUTPUTROOT
  SimulationOptions options(options_file);

  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      std::filesystem::path(options.OUTPUTROOT) /
      ("bendersLog-rank" + std::to_string(world.rank()) + ".txt.");
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  auto log_reports_name =
      std::filesystem::path(options.OUTPUTROOT) / "reportbenders.txt";
  Logger logger;
  Writer writer;

  if (world.rank() == 0) {
    auto logger_factory = FileAndStdoutLoggerFactory(log_reports_name);

    logger = logger_factory.get_logger();
    writer = build_json_writer(options.JSON_FILE, options.RESUME);
    if (Benders::StartUp startup;
        startup.StudyAlreadyAchievedCriterion(options, writer, logger))
      return 0;
    std::ostringstream oss_l = start_message(options, "mpi");
    LOG(INFO) << oss_l.str() << std::endl;
  } else {
    logger = build_void_logger();
    writer = build_void_writer();
  }

  world.barrier();
  Timer timer;
  pBendersBase benders;

  benders =
      std::make_shared<BendersMpi>(benders_options, logger, writer, env, world);
  benders->set_log_file(log_reports_name);

  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);
  benders->launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(timer.elapsed());
  return 0;
}

BendersMainFactory::BendersMainFactory(int argc, char** argv,
                                       const BENDERSMETHOD& method,
                                       mpi::environment& env,
                                       mpi::communicator& world)
    : argc_(argc), argv_(argv), method_(method), penv_(&env), pworld_(&world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }

  options_file_ = std::filesystem::path(argv_[1]);
}
BendersMainFactory::BendersMainFactory(
    int argc, char** argv, const BENDERSMETHOD& method,
    const std::filesystem::path& options_file, mpi::environment& env,
    mpi::communicator& world)
    : argc_(argc),
      argv_(argv),
      method_(method),
      options_file_(options_file),
      penv_(&env),
      pworld_(&world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
}
BendersMainFactory::BendersMainFactory(int argc, char** argv,
                                       const BENDERSMETHOD& method)
    : argc_(argc), argv_(argv), method_(method) {
  usage(argc_);
  options_file_ = std::filesystem::path(argv_[1]);
}
BendersMainFactory::BendersMainFactory(
    int argc, char** argv, const BENDERSMETHOD& method,
    const std::filesystem::path& options_file)
    : argc_(argc), argv_(argv), method_(method), options_file_(options_file) {
  usage(argc);
}
int BendersMainFactory::Run() const {
  if (method_ == BENDERSMETHOD::BENDERSBYBATCH && pworld_->rank() == 0) {
    return RunBendersByBatch(argv_, options_file_, *penv_, *pworld_);
  } else {
    return RunBenders(argv_, options_file_, *penv_, *pworld_);
  }
}