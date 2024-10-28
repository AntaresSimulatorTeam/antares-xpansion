
#include "antares-xpansion/benders/factories/BendersFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"
#include "antares-xpansion/benders/benders_core/MasterUpdate.h"
#include "antares-xpansion/benders/benders_core/StartUp.h"
#include "antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h"
#include "antares-xpansion/benders/benders_mpi/OuterLoopBenders.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/AreaParser.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

BENDERSMETHOD DeduceBendersMethod(size_t coupling_map_size, size_t batch_size,
                                  bool external_loop) {
  if (batch_size == 0 || batch_size == coupling_map_size - 1) {
    if (external_loop) {
      return BENDERSMETHOD::BENDERS_OUTERLOOP;
    } else {
      return BENDERSMETHOD::BENDERS;
    }
  } else {
    if (external_loop) {
      return BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP;
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

  method_ = DeduceBendersMethod(coupling_map.size(), options_.BATCH_SIZE,
                                external_loop);
  context_ = bendersmethod_to_string(method_);

  auto benders_log_console = benders_options.LOG_LEVEL > 0;
  if (pworld_->rank() == 0) {
    auto logger_factory =
        FileAndStdoutLoggerFactory(LogReportsName(), benders_log_console);
    logger_ = logger_factory.get_logger();
    math_log_driver = BuildMathLogger(benders_log_console);
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
  auto outer_loop_input_data = ProcessCriterionInput();
  criterion_computation_ =
      std::make_shared<Benders::Criterion::CriterionComputation>(
          outer_loop_input_data);

  if (pworld_->rank() == 0 && !outer_loop_input_data.OuterLoopData().empty()) {
    AddCriterionOutput(math_log_driver);
  }

  switch (method_) {
    case BENDERSMETHOD::BENDERS:
      benders = std::make_shared<BendersMpi>(benders_options, logger_, writer_,
                                             *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
      benders = std::make_shared<Outerloop::BendersMpiOuterLoop>(
          benders_options, logger_, writer_, *penv_, *pworld_, math_log_driver);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
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
    bool benders_log_console) const {
  const std::filesystem::path output_root(options_.OUTPUTROOT);
  auto math_logs_file = output_root / "benders_solver.log";

  auto math_log_factory =
      MathLoggerFactory(method_, benders_log_console, math_logs_file);

  auto math_log_driver = math_log_factory.get_logger();

  return math_log_driver;
}

void BendersMainFactory::AddCriterionOutput(
    std::shared_ptr<MathLoggerDriver> math_log_driver) {
  const std::filesystem::path output_root(options_.OUTPUTROOT);

  const auto& headers =
      criterion_computation_->getOuterLoopInputData().PatternBodies();
  math_log_driver->add_logger(
      output_root / LOLD_FILE, headers,
      &OuterLoopCurrentIterationData::outer_loop_criterion);

  positive_unsupplied_file_ =
      criterion_computation_->getOuterLoopInputData().PatternsPrefix() + ".txt";
  math_log_driver->add_logger(
      output_root / positive_unsupplied_file_, headers,
      &OuterLoopCurrentIterationData::outer_loop_patterns_values);
}

int BendersMainFactory::RunBenders() {

  try {
    auto benders = PrepareForExecution(false);
    if (benders) {
      benders->launch();
      EndMessage(benders->execution_time());
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

void BendersMainFactory::EndMessage(const double execution_time) {
  std::ostringstream str;
  str << "Optimization results available in : " << options_.JSON_FILE
      << std::endl;
  benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO,
                                   context_);

  str.str("");

  str << bendersmethod_to_string(method_) << " ran in " << execution_time
      << " s" << std::endl;
  benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO,
                                   context_);
}

Benders::Criterion::OuterLoopInputData
BendersMainFactory::ProcessCriterionInput() {
  const auto fpath = std::filesystem::path(options_.INPUTROOT) /
                     options_.OUTER_LOOP_OPTION_FILE;
  // if adequacy_criterion.yml is provided read it
  if (std::filesystem::exists(fpath)) {
    return Benders::Criterion::OuterLoopInputFromYaml().Read(fpath);
  }
  // else compute criterion for all areas!
  else {
    return BuildPatternsUsingAreaFile();
  }
}

Benders::Criterion::OuterLoopInputData
BendersMainFactory::BuildPatternsUsingAreaFile() {
  std::set<std::string> unique_areas = ReadAreaFile();
  Benders::Criterion::OuterLoopInputData ret;
  ret.SetCriterionCountThreshold(1);

  for (const auto& area : unique_areas) {
    Benders::Criterion::OuterLoopSingleInputData singleInputData(
        Benders::Criterion::PositiveUnsuppliedEnergy, area, 1);
    ret.AddSingleData(singleInputData);
  }

  return ret;
}

std::set<std::string> BendersMainFactory::ReadAreaFile() {
  std::set<std::string> unique_areas;
  const auto area_file =
      std::filesystem::path(options_.INPUTROOT) / options_.AREA_FILE;
  const auto area_file_data = AreaParser::ReadAreaFile(area_file);
  if (const auto& msg = area_file_data.error_message; !msg.empty()) {
    benders_loggers_.display_message(msg, LogUtils::LOGLEVEL::WARNING,
                                     context_);
    std::ostringstream ms;
    ms << " Consequently, " << LOLD_FILE
       << " and other criterion based files will not be produced!";

    benders_loggers_.display_message(ms.str(), LogUtils::LOGLEVEL::WARNING,
                                     context_);
    return {};
  }
  return {area_file_data.areas.begin(), area_file_data.areas.end()};
}


int BendersMainFactory::RunExternalLoop() {
  try {
    auto benders = PrepareForExecution(true);
    double tau = 0.5;

    const auto& criterion_input_data =
        criterion_computation_->getOuterLoopInputData();
    std::shared_ptr<Outerloop::IMasterUpdate> master_updater =
        std::make_shared<Outerloop::MasterUpdateBase>(
            benders, tau, criterion_input_data.StoppingThreshold());
    std::shared_ptr<Outerloop::ICutsManager> cuts_manager =
        std::make_shared<Outerloop::CutsManagerRunTime>();

    Outerloop::OuterLoopBenders ext_loop(criterion_input_data.OuterLoopData(),
                                         master_updater, cuts_manager, benders,
                                         *pworld_);
    ext_loop.Run();
    EndMessage(ext_loop.Runtime());
    
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
      penv_(&env),
      pworld_(&world),
      solver_(solver),
      options_(options_file) {
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
