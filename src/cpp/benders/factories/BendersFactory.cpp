
#include "BendersFactory.h"

#include <filesystem>

#include "LoggerFactories.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

BendersSequentialFactory::BendersSequentialFactory(
    const BendersBaseOptions& benders_options, Logger logger, Writer writer,
    const BENDERSMETHOD& method) {
  if (method == BENDERSMETHOD::SEQUENTIAL || method == BENDERSMETHOD::MPI) {
    benders_ =
        std::make_shared<BendersSequential>(benders_options, logger, writer);
  } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
    benders_ =
        std::make_shared<BendersByBatch>(benders_options, logger, writer);
  }
}

pBendersBase BendersSequentialFactory::GetBenders() const { return benders_; }
#ifdef __BENDERSMPI__

int RunMpi(char** argv, const std::filesystem::path& options_file,
           mpi::environment& env, mpi::communicator& world) {
  // Read options, needed to have options.OUTPUTROOT
  SimulationOptions options(options_file);

  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      std::filesystem::path(options.OUTPUTROOT) /
      ("bendersmpiLog-rank" + std::to_string(world.rank()) + ".txt.");
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  auto log_reports_name =
      std::filesystem::path(options.OUTPUTROOT) / "reportbendersmpi.txt";
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

  if (world.size() == 1) {
    std::cout << "Sequential launch" << std::endl;
    LOG(INFO) << "Size is 1. Launching in sequential mode..." << std::endl;

    benders =
        std::make_shared<BendersSequential>(benders_options, logger, writer);
  } else {
    benders = std::make_shared<BendersMpi>(benders_options, logger, writer, env,
                                           world);
  }
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
  //   return RunMpi(argv, argv[1], env, world);
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
  //   return RunMpi(argv, options_file, env, world);
}
#endif  // __BENDERSMPI__
int RunSequential(char** argv, const std::filesystem::path& options_file,
                  const BENDERSMETHOD& method) {
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

  BendersSequentialFactory benders_factory(benders_options, logger, writer,
                                           method);
  auto benders = benders_factory.GetBenders();
  benders->set_log_file(loggerFileName);
  benders->launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(benders->execution_time());

  return 0;
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
#ifdef __BENDERSMPI__
  if (method_ == BENDERSMETHOD::MPI) {
    return RunMpi(argv_, options_file_, *penv_, *pworld_);
  } else if (pworld_->rank() == 0) {
    return RunSequential(argv_, options_file_, method_);
  }
#else
  return RunSequential(argv_, options_file_, method_);
#endif  //__BENDERSMPI__
}