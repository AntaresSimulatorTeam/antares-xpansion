#include <boost/program_options.hpp>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "antares-xpansion/sensitivity/SensitivityFileLogger.h"
#include "antares-xpansion/sensitivity/SensitivityInputReader.h"
#include "antares-xpansion/sensitivity/SensitivityLogger.h"
#include "antares-xpansion/sensitivity/SensitivityMasterLogger.h"
#include "antares-xpansion/sensitivity/SensitivityStudy.h"
namespace po = boost::program_options;

const std::string DEFAULT_SENSITIVITY_OUTPUT_JSON("sensitivity.json");
const std::string DEFAULT_SENSITIVITY_LOG_FILE("sensitivity_log.txt");

std::shared_ptr<SensitivityILogger> build_logger(
    const std::filesystem::path &log_file_path) {
  auto master_logger = std::make_shared<SensitivityMasterLogger>();
  std::shared_ptr<SensitivityILogger> file_logger =
      std::make_shared<SensitivityFileLogger>(log_file_path);
  std::shared_ptr<SensitivityILogger> user_logger =
      std::make_shared<SensitivityLogger>(std::cout);
  master_logger->addLogger(file_logger);
  master_logger->addLogger(user_logger);
  std::shared_ptr<SensitivityILogger> logger = master_logger;
  return logger;
}

int main(int argc, char **argv) {
  try {
    std::string json_input_path;
    std::filesystem::path json_output_path;
    std::string benders_output_path;
    std::string last_master_path;
    std::string basis_path;
    std::string structure_path;
    std::filesystem::path log_path;

    po::options_description desc("Allowed options");

    desc.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>(&json_input_path)->required(),
        "path to the json input file")(
        "output,o", po::value<std::filesystem::path>(&json_output_path),
        "path to the sensitivity json output file")(
        "benders,b", po::value<std::string>(&benders_output_path)->required(),
        "path to the benders json output file")(
        "master,m", po::value<std::string>(&last_master_path)->required(),
        "path to the last master mps file")(
        "basis", po::value<std::string>(&basis_path)->default_value(""),
        "path to the last master optimal basis file")(
        "structure,s", po::value<std::string>(&structure_path)->required(),
        "path to the structure txt file")(
        "log,l", po::value<std::filesystem::path>(&log_path),
        "path to the sensitivity log file");

    po::variables_map opts;
    po::store(po::parse_command_line(argc, argv, desc), opts);

    if (opts.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    po::notify(opts);

    auto sensitivity_input_reader = SensitivityInputReader(
        json_input_path, benders_output_path, last_master_path, basis_path, structure_path);
    SensitivityInputData input_data = sensitivity_input_reader.get_input_data();

    auto input_dir = std::filesystem::path(json_input_path).parent_path();

    if (!opts.count("output")) {
      json_output_path = input_dir / DEFAULT_SENSITIVITY_OUTPUT_JSON;
    }

    if (!opts.count("log")) {
      std::filesystem::path windows_path(json_input_path);
      log_path = input_dir / DEFAULT_SENSITIVITY_LOG_FILE;
    }
    auto logger = build_logger(log_path);

    auto writer = std::make_shared<SensitivityWriter>(json_output_path);
    auto sensitivity_study = SensitivityStudy(input_data, logger, writer);

    sensitivity_study.launch();
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }
}