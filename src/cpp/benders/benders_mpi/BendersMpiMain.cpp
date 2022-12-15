// projet_benders.cpp: defines the entry point  for the console application
//

#include "BendersMpiMain.h"

#include <filesystem>

#include "BendersSequential.h"
#include "LoggerFactories.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

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
int BendersMpiMain(int argc, char** argv, mpi::environment& env,
                   mpi::communicator& world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
  return RunMpi(argv, argv[1], env, world);
}
int BendersMpiMain(int argc, char** argv,
                   const std::filesystem::path& options_file,
                   mpi::environment& env, mpi::communicator& world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
  return RunMpi(argv, options_file, env, world);
}