#include <boost/program_options.hpp>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "SensitivityFileLogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityLogger.h"
#include "SensitivityMasterLogger.h"
#include "SensitivityStudy.h"
#include "helpers/Path.h"
namespace po = boost::program_options;

const std::string DEFAULT_SENSITIVITY_OUTPUT_JSON("sensitivity.json");
const std::string DEFAULT_SENSITIVITY_LOG_FILE("sensitivity_log.txt");

std::shared_ptr<SensitivityILogger> build_logger(
    const std::string &log_file_path) {
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
std::string get_sensitivity_input_dir(const std::string &input_file) {
  size_t last_sep(input_file.find(Path::mSep));

  if (last_sep == std::string::npos) {
    return ".";
  }

  while (true) {
    size_t next_sep = input_file.find(Path::mSep, last_sep + 1);
    if (next_sep == std::string::npos) {
      break;
    } else {
      last_sep = next_sep;
    }
  }

  return (input_file.substr(0, last_sep));
}

int main(int argc, char **argv) {
  try {
    std::string json_input_path;
    std::string json_output_path;
    std::string benders_output_path;
    std::string last_master_path;
    std::string structure_path;
    std::string log_path;

    po::options_description desc("Allowed options");

    desc.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>(&json_input_path)->required(),
        "path to the json input file")(
        "output,o", po::value<std::string>(&json_output_path),
        "path to the sensitivity json output file")(
        "benders,b", po::value<std::string>(&benders_output_path)->required(),
        "path to the benders json output file")(
        "master,m", po::value<std::string>(&last_master_path)->required(),
        "path to the last master mps file")(
        "structure,s", po::value<std::string>(&structure_path)->required(),
        "path to the structure txt file")("log,l",
                                          po::value<std::string>(&log_path),
                                          "path to the sensitivity log file");

    po::variables_map opts;
    po::store(po::parse_command_line(argc, argv, desc), opts);

    if (opts.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    po::notify(opts);

    auto sensitivity_input_reader = SensitivityInputReader(
        json_input_path, benders_output_path, last_master_path, structure_path);
    SensitivityInputData input_data = sensitivity_input_reader.get_input_data();

    if (!opts.count("output")) {
      std::filesystem::path windows_path(json_input_path);
      json_output_path = (Path(get_sensitivity_input_dir(
                              windows_path.make_preferred().string())) /
                          DEFAULT_SENSITIVITY_OUTPUT_JSON)
                             .get_str();
    }

    if (!opts.count("log")) {
      std::filesystem::path windows_path(json_input_path);
      log_path = (Path(get_sensitivity_input_dir(
                      windows_path.make_preferred().string())) /
                  DEFAULT_SENSITIVITY_LOG_FILE)
                     .get_str();
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