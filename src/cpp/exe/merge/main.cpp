// projet_benders.cpp : définit le point d'entrée pour l'application console.
//

#include <filesystem>

#include "JsonWriter.h"
#include "MergeMPS.h"
#include "SimulationOptions.h"
#include "Worker.h"
#include "WriterFactories.h"

#include "logger/User.h"
#include "solver_utils.h"

//@suggest: create and move to standardlp.cpp
// Initialize static member
size_t StandardLp::appendCNT = 0;

int main(int argc, char **argv) {
  usage(argc);
  SimulationOptions options(argv[1]);
  options.print(std::cout);

  Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

  logger->display_message("starting merge_mps");

  Writer writer =
      build_json_writer(std::filesystem::path(options.JSON_FILE), false);
  try {
    MergeMPS merge_mps(options.get_base_options(), logger, writer);
    merge_mps.launch();
  } catch (std::exception &ex) {
    std::string error =
        "Exception raised and program stopped : " + std::string(ex.what());
    logger->display_message(error);
    exit(1);
  }

  return 0;
}
//TODO
/*
 * int main(int argc, char **argv) {
// options.print(std::cout);
usage(argc);
SimulationOptions options(argv[1]);
BendersBaseOptions benders_options(options.get_benders_options());
std::unique_ptr<SimulationOptions> options;
if (argc == 2) {
  options = std::make_unique<SimulationOptions>(argv[1]);
} else {
  options = std::make_unique<SimulationOptions>(argv[1], argv[2]);
}
//options->print(std::cout);
BendersBaseOptions benders_options(options->get_benders_options());

google::InitGoogleLogging(argv[0]);
auto path_to_log =
  std::filesystem::path(options.OUTPUTROOT) / "benderssequentialLog.txt.";
std::filesystem::path(options->OUTPUTROOT) / "benderssequentialLog.txt.";
google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

std::ostringstream oss_l = start_message(options, "Sequential");
std::ostringstream oss_l = start_message(*options, "Sequential");
LOG(INFO) << oss_l.str() << std::endl;

const auto &loggerFileName =
  std::filesystem::path(options.OUTPUTROOT) / "reportbenderssequential.txt";
std::filesystem::path(options->OUTPUTROOT) / "reportbenderssequential.txt";

auto logger_factory = FileAndStdoutLoggerFactory(loggerFileName);

Logger logger = logger_factory.get_logger();
Writer writer = build_json_writer(options.JSON_FILE, options.RESUME);
if (options.RESUME && writer->solution_status() == Output::STATUS_OPTIMAL_C) {
Writer writer = build_json_writer(options->JSON_FILE, options->RESUME);
if (options->RESUME && writer->solution_status() == Output::STATUS_OPTIMAL_C) {
  std::stringstream str;
  str << "Study is already optimal " << std::endl
      << "Optimization results available in : " << options.JSON_FILE;
  << "Optimization results available in : " << options->JSON_FILE;
  logger->display_message(str.str());
  return 0;
}
writer->write_log_level(options.LOG_LEVEL);
writer->write_master_name(options.MASTER_NAME);
writer->write_solver_name(options.SOLVER_NAME);
writer->write_log_level(options->LOG_LEVEL);
writer->write_master_name(options->MASTER_NAME);
writer->write_solver_name(options->SOLVER_NAME);

BendersSequential benders(benders_options, logger, writer);
benders.set_log_file(loggerFileName);
benders.launch();
std::stringstream str;
str << "Optimization results available in : " << options.JSON_FILE;
str << "Optimization results available in : " << options->JSON_FILE;
logger->display_message(str.str());
logger->log_total_duration(benders.execution_time());


*/
/*
 * // Read options, needed to have options.OUTPUTROOT
SimulationOptions options(argv[1]);
std::unique_ptr<SimulationOptions> options;
if (argc == 2) {
  options = std::make_unique<SimulationOptions>(argv[1]);
} else {
  options = std::make_unique<SimulationOptions>(argv[1], argv[2]);
}

BendersBaseOptions benders_options(options.get_benders_options());
BendersBaseOptions benders_options(options->get_benders_options());

gflags::ParseCommandLineFlags(&argc, &argv, true);
google::InitGoogleLogging(argv[0]);
auto path_to_log =
  std::filesystem::path(options.OUTPUTROOT) /
  std::filesystem::path(options->OUTPUTROOT) /
  ("bendersmpiLog-rank" + std::to_string(world.rank()) + ".txt.");
google::SetLogDestination(google::GLOG_INFO, path_to_log.string().c_str());

auto log_reports_name =
  std::filesystem::path(options.OUTPUTROOT) / "reportbendersmpi.txt";
std::filesystem::path(options->OUTPUTROOT) / "reportbendersmpi.txt";
Logger logger;
Writer writer;


auto logger_factory = FileAndStdoutLoggerFactory(log_reports_name);

logger = logger_factory.get_logger();
writer = build_json_writer(options.JSON_FILE, options.RESUME);
  if (options.RESUME &&
  writer = build_json_writer(options->JSON_FILE, options->RESUME);
  if (options->RESUME &&
      writer->solution_status() == Output::STATUS_OPTIMAL_C) {
std::stringstream str;
str << "Study is already optimal " << std::endl
    << "Optimization results available in : " << options.JSON_FILE;
<< "Optimization results available in : " << options->JSON_FILE;
logger->display_message(str.str());
return 0;
  }
  std::ostringstream oss_l = start_message(options, "mpi");
  std::ostringstream oss_l = start_message(*options, "mpi");
  LOG(INFO) << oss_l.str() << std::endl;
  } else {
    logger = build_void_logger();

  }
  benders->set_log_file(log_reports_name);

  writer->write_log_level(options.LOG_LEVEL);
  writer->write_master_name(options.MASTER_NAME);
  writer->write_solver_name(options.SOLVER_NAME);
  writer->write_log_level(options->LOG_LEVEL);
  writer->write_master_name(options->MASTER_NAME);
  writer->write_solver_name(options->SOLVER_NAME);
  benders->launch();
  std::stringstream str;
  str << "Optimization results available in : " << options.JSON_FILE;
  str << "Optimization results available in : " << options->JSON_FILE;
  logger->display_message(str.str());
  logger->log_total_duration(timer.elapsed());
  return 0;

    */