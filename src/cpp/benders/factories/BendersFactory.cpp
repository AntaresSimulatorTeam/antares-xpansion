
#include "BendersFactory.h"

#include "BendersByBatch.h"
#include "BendersMpiOuterLoop.h"
#include "LogUtils.h"
#include "LoggerFactories.h"
#include "MasterUpdate.h"
#include "OuterLoopBenders.h"
#include "StartUp.h"
#include "Timer.h"
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

pBendersBase BendersMainFactory::PrepareForExecution(bool external_loop) {
  pBendersBase benders;
  std::shared_ptr<MathLoggerDriver> math_log_driver;

  BendersBaseOptions benders_options(options_.get_benders_options());
  benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = external_loop;


  const auto coupling_map = build_input(benders_options.STRUCTURE_FILE);
  auto outer_loop_input_data = ProcessCriterionInput(coupling_map);
  criterion_computation_ =
      std::make_shared<Outerloop::CriterionComputation>(outer_loop_input_data);

  const auto method = DeduceBendersMethod(coupling_map.size(),
                                          options_.BATCH_SIZE, external_loop);

  if (pworld_->rank() == 0) {
    auto benders_log_console = benders_options.LOG_LEVEL > 0;
    auto logger_factory =
        FileAndStdoutLoggerFactory(LogReportsName(), benders_log_console);
    logger_ = logger_factory.get_logger();

    math_log_driver = BuildMathLogger(method, benders_log_console);

    writer_ = build_json_writer(options_.JSON_FILE, options_.RESUME);
    if (Benders::StartUp startup;
        startup.StudyAlreadyAchievedCriterion(options_, writer_, logger_))
      return nullptr;
  } else {
    logger_ = build_void_logger();
    writer_ = build_void_writer();
    math_log_driver = MathLoggerFactory::get_void_logger();
  }

  benders_loggers_.AddLogger(logger_);
  benders_loggers_.AddLogger(math_log_driver);
  switch (method) {
    case BENDERSMETHOD::BENDERS:
      benders = std::make_shared<BendersMpi>(benders_options, logger_, writer_,
                                             *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_EXTERNAL_LOOP:
      benders = std::make_shared<Outerloop::BendersMpiOuterLoop>(
          benders_options, logger_, writer_, *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
    case BENDERSMETHOD::BENDERS_BY_BATCH_EXTERNAL_LOOP:
      benders = std::make_shared<BendersByBatch>(
          benders_options, logger_, writer_, *penv_, *pworld_, math_log_driver);
      break;
  }

  benders->set_input_map(coupling_map);
  std::ostringstream oss_l = start_message(options_, benders->BendersName());
  oss_l << std::endl;
  benders_loggers_.display_message(oss_l.str());

  if (benders_options.LOG_LEVEL > 1) {
    auto solver_log = std::filesystem::path(options_.OUTPUTROOT) /
                      (std::string("solver_log_proc_") +
                       std::to_string(pworld_->rank()) + ".txt");

    benders->set_solver_log_file(solver_log);
  }
  writer_->write_log_level(options_.LOG_LEVEL);
  writer_->write_master_name(options_.MASTER_NAME);
  writer_->write_solver_name(options_.SOLVER_NAME);
  benders->setCriterionsComputation(criterion_computation_);
  return benders;
}

std::shared_ptr<MathLoggerDriver> BendersMainFactory::BuildMathLogger(
    const BENDERSMETHOD& method, bool benders_log_console) const {
  auto math_logs_file =
      std::filesystem::path(options_.OUTPUTROOT) / "benders_solver.log";
  auto math_log_factory =
      MathLoggerFactory(method, benders_log_console, math_logs_file);

  auto math_log_driver = math_log_factory.get_logger();

  const auto& headers =
      criterion_computation_->getOuterLoopInputData().PatternBodies();
  math_log_driver->add_logger(
      std::filesystem::path(options_.OUTPUTROOT) / "LOLD.txt", headers,
      &OuterLoopCurrentIterationData::outer_loop_criterion);
  math_log_driver->add_logger(
      std::filesystem::path(options_.OUTPUTROOT) /
          (criterion_computation_->getOuterLoopInputData().PatternsPrefix() +
           ".txt"),
      headers, &OuterLoopCurrentIterationData::outer_loop_patterns_values);
  return math_log_driver;
}

int BendersMainFactory::RunBenders() {
  // Read options, needed to have options.OUTPUTROOT

  try {
    auto benders = PrepareForExecution(false);
    if (benders) {
      benders->launch();

      std::stringstream str;
      str << "Optimization results available in : " << options_.JSON_FILE
          << std::endl;
      benders_loggers_.display_message(str.str());

      str.str("");
      str << "Benders ran in " << benders->execution_time() << " s"
          << std::endl;
      benders_loggers_.display_message(str.str());
    }

  } catch (std::exception& e) {
    std::ostringstream msg;
    msg << "error: " << e.what() << std::endl;
    benders_loggers_.display_message(msg.str());
    mpi::environment::abort(1);
  } catch (...) {
    std::ostringstream msg;
    msg << "Exception of unknown type!" << std::endl;
    benders_loggers_.display_message(msg.str());
    mpi::environment::abort(1);
  }
  return 0;
}

Outerloop::OuterLoopInputData BendersMainFactory::ProcessCriterionInput(
    const CouplingMap& couplingMap) {
  const auto fpath = std::filesystem::path(options_.INPUTROOT) /
                     options_.OUTER_LOOP_OPTION_FILE;
  // if adequacy_criterion.yml is provided read it
  if (std::filesystem::exists(fpath)) {
    return Outerloop::OuterLoopInputFromYaml().Read(fpath);
  }
  // else compute criterion for all areas!
  else {
    // convert benders_loggers_ to shared_ptr
    //    SolverFactory factory(benders_loggers_);
    SolverFactory factory(logger_);
    auto solver_log_manager = SolverLogManager(LogReportsName());
    auto solver = factory.create_solver(
        options_.SOLVER_NAME, SOLVER_TYPE::CONTINUOUS, solver_log_manager);
    solver->set_threads(1);

    auto first_subproblem_pair = std::find_if_not(
        couplingMap.begin(), couplingMap.end(),
        [this](const auto& in) { return in.first == options_.MASTER_NAME; });
    if (first_subproblem_pair != couplingMap.end()) {
      solver->read_prob_mps(std::filesystem::path(options_.INPUTROOT) /
                            first_subproblem_pair->first);
    } else {
      std::ostringstream stream;
      auto log_location = LOGLOCATION;
      stream << "Could not find any Subproblem in structure file "
             << options_.STRUCTURE_FILE << std::endl;
      benders_loggers_.display_message(log_location + stream.str());
      throw InvalidStructureFile(
          PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Benders"), stream.str(),
          log_location);
    }
  }
}

int BendersMainFactory::RunExternalLoop() {
  try {
    auto benders = PrepareForExecution(true);
    double tau = 0.5;

    std::shared_ptr<Outerloop::IMasterUpdate> master_updater =
        std::make_shared<Outerloop::MasterUpdateBase>(
            benders, tau,
            criterion_computation_->getOuterLoopInputData()
                .StoppingThreshold());
    std::shared_ptr<Outerloop::ICutsManager> cuts_manager =
        std::make_shared<Outerloop::CutsManagerRunTime>();

    Outerloop::OuterLoopBenders ext_loop(*criterion_computation_,
                                         master_updater, cuts_manager, benders,
                                         *pworld_);
    ext_loop.Run();

    } catch (std::exception& e) {
    std::ostringstream msg;
    msg << "error: " << e.what() << std::endl;
    benders_loggers_.display_message(msg.str());
    mpi::environment::abort(1);
  } catch (...) {
    std::ostringstream msg;
    msg << "Exception of unknown type!" << std::endl;
    benders_loggers_.display_message(msg.str());
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

  options_.read(std::filesystem::path(argv_[1]));
}

BendersMainFactory::BendersMainFactory(
    int argc, char** argv, const std::filesystem::path& options_file,
    mpi::environment& env, mpi::communicator& world, const SOLVER& solver)
    : argv_(argv),
      options_(options_file),
      penv_(&env),
      pworld_(&world),
      solver_(solver) {
  // First check usage (options are given)
  if (world.rank() == 0) {
    usage(argc);
  }
}
std::filesystem::path BendersMainFactory::LogReportsName() const {
  return std::filesystem::path(options_.OUTPUTROOT) / "reportbenders.txt";
}

int BendersMainFactory::Run() {
  if (solver_ == SOLVER::BENDERS) {
    return RunBenders();
  } else {
    return RunExternalLoop();
  }
}
