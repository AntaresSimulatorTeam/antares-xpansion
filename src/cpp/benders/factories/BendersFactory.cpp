
#include "antares-xpansion/benders/factories/BendersFactory.h"

#include <filesystem>

#include "antares-xpansion/benders/benders_by_batch/BendersByBatch.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/MasterUpdate.h"
#include "antares-xpansion/benders/benders_core/StartUp.h"
#include "antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h"
#include "antares-xpansion/benders/benders_mpi/OuterLoopBenders.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/helpers/AreaParser.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

BENDERSMETHOD DeduceBendersMethod(size_t coupling_map_size, size_t batch_size,
                                  bool outer_loop) {
  if (batch_size == 0 || batch_size == coupling_map_size - 1) {
    if (outer_loop) {
      return BENDERSMETHOD::BENDERS_OUTERLOOP;
    } else {
      return BENDERSMETHOD::BENDERS;
    }
  } else {
    if (outer_loop) {
      return BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP;
    } else {
      return BENDERSMETHOD::BENDERS_BY_BATCH;
    }
  }
}

void BendersMainFactory::PrepareForExecution(bool outer_loop) {
  BendersBaseOptions benders_options(options_.get_benders_options());
  benders_options.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = outer_loop;

  SetupLoggerAndOutputWriter(benders_options);

  const auto coupling_map = CouplingMapGenerator::BuildInput(
      benders_options.STRUCTURE_FILE,
      std::make_shared<BendersLoggerBase>(benders_loggers_), "Benders");

  method_ =
      DeduceBendersMethod(coupling_map.size(), options_.BATCH_SIZE, outer_loop);
  context_ = bendersmethod_to_string(method_);

  criterion_input_holder_ = ProcessCriterionInput();

  if (pworld_->rank() == 0) {
    if (Benders::StartUp startup;
        startup.StudyAlreadyAchievedCriterion(options_, writer_, logger_)) {
      return;
    }
    if (!isCriterionListEmpty()) {
      AddCriterionOutputs();
    }
  }

  ConfigureBenders(benders_options, coupling_map);
  ConfigureSolverLog();
}

void BendersMainFactory::ConfigureSolverLog() {
  if (options_.LOG_LEVEL > 1) {
    auto solver_log = std::filesystem::path(options_.OUTPUTROOT) /
                      (std::string("solver_log_proc_") +
                       std::to_string(pworld_->rank()) + ".txt");

    benders_->set_solver_log_file(solver_log);
  }
}

void BendersMainFactory::ConfigureBenders(
    const BendersBaseOptions& benders_options,
    const CouplingMap& coupling_map) {
  switch (method_) {
    case BENDERSMETHOD::BENDERS:
      benders_ =
          std::make_shared<BendersMpi>(benders_options, logger_, writer_,
                                       *penv_, *pworld_, math_log_driver_);
      break;
    case BENDERSMETHOD::BENDERS_OUTERLOOP:
      benders_ = std::make_shared<Outerloop::BendersMpiOuterLoop>(
          benders_options, logger_, writer_, *penv_, *pworld_,
          math_log_driver_);
      break;
    case BENDERSMETHOD::BENDERS_BY_BATCH:
    case BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP:
      benders_ =
          std::make_shared<BendersByBatch>(benders_options, logger_, writer_,
                                           *penv_, *pworld_, math_log_driver_);
      break;
  }

  benders_->set_input_map(coupling_map);
  benders_->setCriterionComputationInputs(std::visit(
      [](auto&& the_variant) {
        return static_cast<Benders::Criterion::CriterionInputData>(the_variant);
      },
      criterion_input_holder_));
}

void BendersMainFactory::SetupLoggerAndOutputWriter(
    const BendersBaseOptions& benders_options) {
  auto benders_log_console = benders_options.LOG_LEVEL > 0;
  if (pworld_->rank() == 0) {
    auto logger_factory =
        FileAndStdoutLoggerFactory(LogReportsName(), benders_log_console);
    logger_ = logger_factory.get_logger();
    math_log_driver_ = BuildMathLogger(benders_log_console);
    writer_ = build_json_writer(options_.JSON_FILE, options_.RESUME);
  } else {
    logger_ = build_void_logger();
    writer_ = build_void_writer();
    math_log_driver_ = MathLoggerFactory::get_void_logger();
  }
  benders_loggers_.AddLogger(logger_);
  benders_loggers_.AddLogger(math_log_driver_);
  writer_->write_log_level(options_.LOG_LEVEL);
  writer_->write_master_name(options_.MASTER_NAME);
  writer_->write_solver_name(options_.SOLVER_NAME);
}

bool BendersMainFactory::isCriterionListEmpty() const {
  return std::visit(
    [](auto &&the_variant) { return the_variant.Criteria().empty(); },
    criterion_input_holder_);
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

void BendersMainFactory::AddCriterionOutputs() {
  const std::filesystem::path output_root(options_.OUTPUTROOT);

  const auto& headers =
      std::visit([](auto&& the_variant) { return the_variant.PatternBodies(); },
                 criterion_input_holder_);
  math_log_driver_->add_logger(
      output_root / LOLD_FILE, headers,
      &CriteriaCurrentIterationData::criteria);

  positive_unsupplied_file_ = std::visit(
      [](auto&& the_variant) { return the_variant.PatternsPrefix() + ".txt"; },
      criterion_input_holder_);
  math_log_driver_->add_logger(
      output_root / positive_unsupplied_file_, headers,
      &CriteriaCurrentIterationData::patterns_values);
}

int BendersMainFactory::RunBenders() {

  try {
    PrepareForExecution(false);
    if (benders_) {
      StartMessage();
      benders_->launch();
      EndMessage(benders_->execution_time());
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

void BendersMainFactory::StartMessage() {
  std::ostringstream oss_l;
  oss_l << "Starting " << context_ << std::endl;
  options_.print(oss_l);
  oss_l << std::endl;
  benders_loggers_.display_message(oss_l.str());
}

void BendersMainFactory::EndMessage(const double execution_time) {
  std::ostringstream str;
  str << "Optimization results available in : " << options_.JSON_FILE
      << std::endl;
  benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO,
                                   context_);

  str.str("");

  str << context_ << " ran in " << execution_time << " s" << std::endl;
  benders_loggers_.display_message(str.str(), LogUtils::LOGLEVEL::INFO,
                                   context_);
}

std::variant<Benders::Criterion::CriterionInputData,
             Benders::Criterion::OuterLoopCriterionInputData>
BendersMainFactory::ProcessCriterionInput() {
  const auto fpath = std::filesystem::path(options_.INPUTROOT) /
                     options_.OUTER_LOOP_OPTION_FILE;
  // if adequacy_criterion.yml is provided read it
  if ((method_ == BENDERSMETHOD::BENDERS_OUTERLOOP ||
       method_ == BENDERSMETHOD::BENDERS_BY_BATCH_OUTERLOOP) &&
      std::filesystem::exists(fpath)) {
    return Benders::Criterion::CriterionInputFromYaml().Read(fpath);
  }
  // else compute criterion for all areas!
  else {
    return BuildPatternsUsingAreaFile();
  }
}

Benders::Criterion::CriterionInputData
BendersMainFactory::BuildPatternsUsingAreaFile() {
  std::set<std::string> unique_areas = ReadAreaFile();
  Benders::Criterion::CriterionInputData ret;
  ret.SetCriterionCountThreshold(1);

  for (const auto& area : unique_areas) {
    Benders::Criterion::CriterionSingleInputData singleInputData(
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
    PrepareForExecution(true);
    double tau = 0.5;
    const auto& outer_loop_inputs =
        std::get<Benders::Criterion::OuterLoopCriterionInputData>(
            criterion_input_holder_);
    std::shared_ptr<Outerloop::IMasterUpdate> master_updater =
        std::make_shared<Outerloop::MasterUpdateBase>(
            benders_, tau, outer_loop_inputs.StoppingThreshold());
    std::shared_ptr<Outerloop::ICutsManager> cuts_manager =
        std::make_shared<Outerloop::CutsManagerRunTime>();

    Outerloop::OuterLoopBenders ext_loop(outer_loop_inputs.Criteria(),
                                         master_updater, cuts_manager, benders_,
                                         *pworld_);
    StartMessage();
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
