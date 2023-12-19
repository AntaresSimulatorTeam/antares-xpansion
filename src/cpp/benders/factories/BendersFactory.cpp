
#include "BendersFactory.h"

#include <filesystem>

#include "LogUtils.h"
#include "LoggerFactories.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

int RunBenders(char** argv, const std::filesystem::path& options_file,
               mpi::environment& env, mpi::communicator& world,
               const BENDERSMETHOD& method) {
  // Read options, needed to have options.OUTPUTROOT
  BendersLoggerBase benders_loggers;
  Logger logger;
  std::shared_ptr<MathLoggerDriver> math_log_driver;

  try {
    /* code */

    SimulationOptions options(options_file);

    BendersBaseOptions benders_options(options.get_benders_options());

    google::InitGoogleLogging(argv[0]);
    auto path_to_log =
        std::filesystem::path(options.OUTPUTROOT) /
        ("bendersLog-rank" + std::to_string(world.rank()) + ".txt.");
    google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

    auto log_reports_name =
        std::filesystem::path(options.OUTPUTROOT) / "reportbenders.txt";

    auto math_logs_file =
        std::filesystem::path(options.OUTPUTROOT) / "benders_solver.log";

    Writer writer;

    if (world.rank() == 0) {
      auto logger_factory = FileAndStdoutLoggerFactory(
          log_reports_name, benders_options.EXPERT_LOGS);
      auto math_log_factory = MathLoggerFactory(
          method, benders_options.EXPERT_LOGS, math_logs_file);

      logger = logger_factory.get_logger();
      math_log_driver = math_log_factory.get_logger();
      writer = build_json_writer(options.JSON_FILE, options.RESUME);
      if (Benders::StartUp startup;
          startup.StudyAlreadyAchievedCriterion(options, writer, logger))
        return 0;
    } else {
      logger = build_void_logger();
      writer = build_void_writer();
      math_log_driver = MathLoggerFactory::get_void_logger();
    }
    benders_loggers.AddLogger(logger);
    benders_loggers.AddLogger(math_log_driver);
    pBendersBase benders;
    if (method == BENDERSMETHOD::BENDERS) {
      benders = std::make_shared<BendersMpi>(benders_options, logger, writer,
                                             env, world, math_log_driver);
    } else if (method == BENDERSMETHOD::BENDERSBYBATCH) {
      benders = std::make_shared<BendersByBatch>(
          benders_options, logger, writer, env, world, math_log_driver);
    } else {
      auto err_msg = "Error only benders or benders-by-batch allowed!";
      benders_loggers.display_message(err_msg);
      std::exit(1);
    }
    std::ostringstream oss_l = start_message(options, benders->BendersName());
    oss_l << std::endl;
    benders_loggers.display_message(oss_l.str());

    auto solver_log = std::filesystem::path(options.OUTPUTROOT) /
                      (std::string("solver_log_proc_") +
                       std::to_string(world.rank()) + ".txt");

    benders->set_solver_log_file(solver_log);

    writer->write_log_level(options.LOG_LEVEL);
    writer->write_master_name(options.MASTER_NAME);
    writer->write_solver_name(options.SOLVER_NAME);
    benders->launch();
    std::stringstream str;
    str << "Optimization results available in : " << options.JSON_FILE;
    benders_loggers.display_message(str.str());
    logger->log_total_duration(benders->execution_time());
  } catch (std::exception& e) {
    std::ostringstream msg;
    msg << "error: " << e.what() << std::endl;
    benders_loggers.display_message(msg.str());
    mpi::environment::abort(1);
    return 1;
  } catch (...) {
    std::ostringstream msg;
    msg << "Exception of unknown type!" << std::endl;
    benders_loggers.display_message(msg.str());
    mpi::environment::abort(1);
    return 1;
  }
  return 0;
}

BendersMainFactory::BendersMainFactory(int argc, char** argv,
                                       const BENDERSMETHOD& method,
                                       mpi::environment& env,
                                       mpi::communicator& world)
    : argv_(argv), method_(method), penv_(&env), pworld_(&world) {
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
    : argv_(argv),
      method_(method),
      options_file_(options_file),
      penv_(&env),
      pworld_(&world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
}
int BendersMainFactory::Run() const {
  return RunBenders(argv_, options_file_, *penv_, *pworld_, method_);
}
