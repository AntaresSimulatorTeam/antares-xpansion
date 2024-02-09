
#include <filesystem>

#include "BendersByBatch.h"
#include "BendersSequential.h"
#include "ILogger.h"
#include "BendersFactory.h"
#include "LogUtils.h"
#include "LoggerFactories.h"
#include "OuterLoop.h"
#include "OutputWriter.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

BENDERSMETHOD DeduceBendersMethod(size_t coupling_map_size, size_t batch_size) {
  auto method = (batch_size == 0 || batch_size == coupling_map_size - 1)
                    ? BENDERSMETHOD::BENDERS
                    : BENDERSMETHOD::BENDERSBYBATCH;

  return method;
}

pBendersBase PrepareForExecution(BendersLoggerBase& benders_loggers,
                                 const SimulationOptions& options,
                                 const char* argv0, mpi::environment& env,
                                 mpi::communicator& world) {
  pBendersBase benders;
  Logger logger;
  std::shared_ptr<MathLoggerDriver> math_log_driver;

  BendersBaseOptions benders_options(options.get_benders_options());

  google::InitGoogleLogging(argv0);
  auto path_to_log =
      std::filesystem::path(options.OUTPUTROOT) /
      ("bendersLog-rank" + std::to_string(world.rank()) + ".txt.");
  google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

  auto log_reports_name =
      std::filesystem::path(options.OUTPUTROOT) / "reportbenders.txt";

  auto math_logs_file =
      std::filesystem::path(options.OUTPUTROOT) / "benders_solver.log";

  Writer writer;
  const auto coupling_map = build_input(benders_options.STRUCTURE_FILE);
  const auto method =
      DeduceBendersMethod(coupling_map.size(), options.BATCH_SIZE);

  if (world.rank() == 0) {
    auto benders_log_console = benders_options.LOG_LEVEL > 0;
    auto logger_factory =
        FileAndStdoutLoggerFactory(log_reports_name, benders_log_console);
    auto math_log_factory =
        MathLoggerFactory(method, benders_log_console, math_logs_file);

    logger = logger_factory.get_logger();
    math_log_driver = math_log_factory.get_logger();
    writer = build_json_writer(options.JSON_FILE, options.RESUME);
    if (Benders::StartUp startup;
        startup.StudyAlreadyAchievedCriterion(options, writer, logger))
      return nullptr;
  } else {
    logger = build_void_logger();
    writer = build_void_writer();
    math_log_driver = MathLoggerFactory::get_void_logger();
  }

  benders_loggers.AddLogger(logger);
  benders_loggers.AddLogger(math_log_driver);
  switch (method) {
    case BENDERSMETHOD::BENDERS:
      benders = std::make_shared<BendersMpi>(benders_options, logger, writer,
                                             env, world, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERSBYBATCH:
      benders = std::make_shared<BendersByBatch>(
          benders_options, logger, writer, env, world, math_log_driver);
      break;
  }
  benders->set_input_map(coupling_map);
  std::ostringstream oss_l = start_message(options, benders->BendersName());
  oss_l << std::endl;
  benders_loggers.display_message(oss_l.str());

  if (benders_options.LOG_LEVEL > 1) {
    auto solver_log = std::filesystem::path(options.OUTPUTROOT) /
                      (std::string("solver_log_proc_") +
                       std::to_string(world.rank()) + ".txt");

    benders->set_solver_log_file(solver_log);
  }
  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);
  return benders;
  }


int RunBenders(char** argv, const std::filesystem::path& options_file,
               mpi::environment& env, mpi::communicator& world) {
  // Read options, needed to have options.OUTPUTROOT
  BendersLoggerBase benders_loggers;

  try {
    SimulationOptions options(options_file);
    auto benders = PrepareForExecution(benders_loggers, options, argv[0], env,
                                       world);
    if (benders) {
      benders->launch();

      std::stringstream str;
      str << "Optimization results available in : " << options.JSON_FILE
          << std::endl;
      benders_loggers.display_message(str.str());

      str.str("");
      str << "Benders ran in " << benders->execution_time() << " s"
          << std::endl;
      benders_loggers.display_message(str.str());
    }

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
int RunExternalLoop_(char** argv, const std::filesystem::path& options_file,
                     mpi::environment& env, mpi::communicator& world) {
  // Read options, needed to have options.OUTPUTROOT
  BendersLoggerBase benders_loggers;

  try {
    SimulationOptions options(options_file);
    auto benders = PrepareForExecution(benders_loggers, options, argv[0], env,
                                       world);
    double threshold = 5684;
    double epsilon = 1e-2;
    double lambda_min = 0;
    double lambda = 16;
    double lambda_max = 17;
    double tau = 0.5;
    std::shared_ptr<IOuterLoopCriterion> criterion =
        std::make_shared<OuterloopCriterionLOL>(threshold, epsilon);
    std::shared_ptr<IMasterUpdate> master_updater =
        // std::make_shared<MasterUpdateBase>(benders, lambda, lambda_min,
        //                                    lambda_max, tau);
        std::make_shared<MasterUpdateBase>(benders, tau);
    std::shared_ptr<ICutsManager> cuts_manager =
        std::make_shared<CutsManagerRunTime>();

    OuterLoop ext_loop(criterion, master_updater, cuts_manager, benders);
    ext_loop.Run();

    // benders->launch();

    // std::stringstream str;
    // str << "Optimization results available in : " << options.JSON_FILE
    //     << std::endl;
    // benders_loggers.display_message(str.str());

    // str.str("");
    // str << "Benders ran in " << benders->execution_time() << " s" <<
    // std::endl; benders_loggers.display_message(str.str());

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

                                       mpi::environment& env,
                                       mpi::communicator& world)
    : argv_(argv), penv_(&env), pworld_(&world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }

  options_file_ = std::filesystem::path(argv_[1]);
}

BendersMainFactory::BendersMainFactory(
    int argc, char** argv, const std::filesystem::path& options_file,
    mpi::environment& env, mpi::communicator& world)
    : argv_(argv), options_file_(options_file), penv_(&env), pworld_(&world) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
}

int BendersMainFactory::Run() const {
  return RunBenders(argv_, options_file_, *penv_, *pworld_);
}

int BendersMainFactory::RunExternalLoop() const {
  return RunExternalLoop_(argv_, options_file_, *penv_, *pworld_);
}
