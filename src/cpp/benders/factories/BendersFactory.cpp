
#include "BendersFactory.h"

#include <filesystem>

#include "BendersByBatch.h"
#include "BendersMpiOuterLoop.h"
#include "BendersSequential.h"
#include "ILogger.h"
#include "LogUtils.h"
#include "LoggerFactories.h"
#include "MasterUpdate.h"
#include "OuterLoopBenders.h"
#include "OutputWriter.h"
#include "StartUp.h"
#include "Timer.h"
#include "Worker.h"
#include "WriterFactories.h"

BENDERSMETHOD DeduceBendersMethod(size_t coupling_map_size, size_t batch_size,
                                  bool external_loop) {
  if (batch_size == 0 || batch_size == coupling_map_size - 1) {
    if (external_loop) {
      return BENDERSMETHOD::BENDERS_EXTERNAL_LOOP;
    } else {
      return BENDERSMETHOD::BENDERS;
    }
  } else {
    if (external_loop) {
      return BENDERSMETHOD::BENDERS_BY_BATCH_EXTERNAL_LOOP;
    } else {
      return BENDERSMETHOD::BENDERS_BY_BATCH;
    }
  }
}

pBendersBase BendersMainFactory::PrepareForExecution(
    BendersLoggerBase& benders_loggers, const SimulationOptions& options,
    bool external_loop,
    std::shared_ptr<Outerloop::CriterionComputation> criterion_computation)
    const {
  pBendersBase benders;
  Logger logger;
  std::shared_ptr<MathLoggerDriver> math_log_driver;


  BendersBaseOptions benders_options(options.get_benders_options());
  benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = external_loop;

  auto log_reports_name =
      std::filesystem::path(options.OUTPUTROOT) / "reportbenders.txt";

  auto math_logs_file =
      std::filesystem::path(options.OUTPUTROOT) / "benders_solver.log";

  Writer writer;
  const auto coupling_map = build_input(benders_options.STRUCTURE_FILE);
  const auto method = DeduceBendersMethod(coupling_map.size(),
                                          options.BATCH_SIZE, external_loop);

  if (pworld_->rank() == 0) {
    auto benders_log_console = benders_options.LOG_LEVEL > 0;
    auto logger_factory =
        FileAndStdoutLoggerFactory(log_reports_name, benders_log_console);
    logger = logger_factory.get_logger();

    math_log_driver =
        BuildMathLogger(options, criterion_computation, math_logs_file, method,
                        benders_log_console);

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
                                             *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_EXTERNAL_LOOP:
      benders = std::make_shared<Outerloop::BendersMpiOuterLoop>(
          benders_options, logger, writer, *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
    case BENDERSMETHOD::BENDERS_BY_BATCH_EXTERNAL_LOOP:
      benders = std::make_shared<BendersByBatch>(
          benders_options, logger, writer, *penv_, *pworld_, math_log_driver);
      break;
  }

  benders->set_input_map(coupling_map);
  std::ostringstream oss_l = start_message(options, benders->BendersName());
  oss_l << std::endl;
  benders_loggers.display_message(oss_l.str());

  if (benders_options.LOG_LEVEL > 1) {
    auto solver_log = std::filesystem::path(options.OUTPUTROOT) /
                      (std::string("solver_log_proc_") +
                       std::to_string(pworld_->rank()) + ".txt");

    benders->set_solver_log_file(solver_log);
  }
  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);
  benders->setCriterionsComputation(criterion_computation);
  return benders;
}
std::shared_ptr<MathLoggerDriver> BendersMainFactory::BuildMathLogger(
    const SimulationOptions& options,
    const std::shared_ptr<Outerloop::CriterionComputation>
        criterion_computation,
    const std::filesystem::path& math_logs_file, const BENDERSMETHOD& method,
    bool benders_log_console) const {
  auto math_log_factory =
      MathLoggerFactory(method, benders_log_console, math_logs_file);

  auto math_log_driver = math_log_factory.get_logger();

  const auto& headers =
      criterion_computation->getOuterLoopInputData().PatternBodies();
  math_log_driver->add_logger(
      std::filesystem::path(options.OUTPUTROOT) / "LOLD.txt", headers,
      &OuterLoopCurrentIterationData::outer_loop_criterion);
  math_log_driver->add_logger(
      std::filesystem::path(options.OUTPUTROOT) /
          (criterion_computation->getOuterLoopInputData().PatternsPrefix() +
           ".txt"),
      headers, &OuterLoopCurrentIterationData::outer_loop_patterns_values);
  return math_log_driver;
}

int BendersMainFactory::RunBenders() const {
  // Read options, needed to have options.OUTPUTROOT
  BendersLoggerBase benders_loggers;

  try {
    SimulationOptions options(options_file_);
    auto benders = PrepareForExecution(benders_loggers, options, false);
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
  } catch (...) {
    std::ostringstream msg;
    msg << "Exception of unknown type!" << std::endl;
    benders_loggers.display_message(msg.str());
    mpi::environment::abort(1);
  }
  return 0;
}
int BendersMainFactory::RunExternalLoop() const {
  BendersLoggerBase benders_loggers;

  try {
    SimulationOptions options(options_file_);
    auto outer_loop_input_data = Outerloop::OuterLoopInputFromYaml().Read(
        std::filesystem::path(options.INPUTROOT) /
        options.OUTER_LOOP_OPTION_FILE);
    auto criterion_computation =
        std::make_shared<Outerloop::CriterionComputation>(
            outer_loop_input_data);
    auto benders = PrepareForExecution(benders_loggers, options, true,
                                       criterion_computation);

    double tau = 0.5;
    double epsilon_lambda = 0.1;

    std::shared_ptr<Outerloop::IMasterUpdate> master_updater =
        std::make_shared<Outerloop::MasterUpdateBase>(
            benders, tau, outer_loop_input_data.StoppingThreshold());
    std::shared_ptr<Outerloop::ICutsManager> cuts_manager =
        std::make_shared<Outerloop::CutsManagerRunTime>();

    Outerloop::OuterLoopBenders ext_loop(*criterion_computation, master_updater,
                                         cuts_manager, benders, *pworld_);
    ext_loop.Run();

    } catch (std::exception& e) {
    std::ostringstream msg;
    msg << "error: " << e.what() << std::endl;
    benders_loggers.display_message(msg.str());
    mpi::environment::abort(1);
  } catch (...) {
    std::ostringstream msg;
    msg << "Exception of unknown type!" << std::endl;
    benders_loggers.display_message(msg.str());
    mpi::environment::abort(1);
  }
  return 0;
}

BendersMainFactory::BendersMainFactory(int argc, char** argv,
                                       mpi::environment& env,
                                       mpi::communicator& world,
                                       const SOLVER& solver)
    : argv_(argv), penv_(&env), pworld_(&world), solver_(solver) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }

  options_file_ = std::filesystem::path(argv_[1]);
}

BendersMainFactory::BendersMainFactory(
    int argc, char** argv, const std::filesystem::path& options_file,
    mpi::environment& env, mpi::communicator& world, const SOLVER& solver)
    : argv_(argv), options_file_(options_file),
      penv_(&env),
      pworld_(&world),
      solver_(solver) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
}
int BendersMainFactory::Run() const {
  if (solver_ == SOLVER::BENDERS) {
    return RunBenders();
  } else {
    return RunExternalLoop();
  }
}
