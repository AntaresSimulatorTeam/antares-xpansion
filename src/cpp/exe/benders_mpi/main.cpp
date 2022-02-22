// projet_benders.cpp: defines the entry point  for the console application
//

#include "Worker.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "BendersMPI.h"
#include "BendersSequential.h"
#include "LoggerFactories.h"
#include "OutputWriter.h"
#include "Timer.h"
#include "WriterFactories.h"
#include "helpers/Path.h"

int main(int argc, char **argv) {
  mpi::environment env(argc, argv);
  mpi::communicator world;

  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }

  // Read options, needed to have options.OUTPUTROOT
  SimulationOptions options(argv[1]);

  BendersBaseOptions benders_options(options.get_benders_options());

  if (world.rank() > options.SLAVE_NUMBER + 1 && options.SLAVE_NUMBER != -1) {
    std::cout << "You need to have at least one slave by thread" << std::endl;
    exit(1);
  }

  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  auto path_to_log =
      (Path(options.OUTPUTROOT) /
       ("bendersmpiLog-rank" + std::to_string(world.rank()) + ".txt."))
          .get_str();
  google::SetLogDestination(google::GLOG_INFO, path_to_log.c_str());

  std::string log_reports_name =
      (Path(options.OUTPUTROOT) / "reportbendersmpi.txt").get_str();
  Logger logger;
  Writer writer;

  FILE *fp = NULL;
  if (world.rank() == 0) {
    auto logger_factory = FileAndStdoutLoggerFactory(log_reports_name);

    logger = logger_factory.get_logger();
    fp = logger_factory.get_file_pointer();
    writer = build_json_writer(options.JSON_FILE);
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
  benders->set_fp(fp);
  benders->launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(timer.elapsed());
  return 0;
}
