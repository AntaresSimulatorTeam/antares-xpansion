#include <boost/program_options.hpp>
#include <iostream>

#include "SensitivityFileLogger.h"
#include "SensitivityInputReader.h"
#include "SensitivityMasterLogger.h"
#include "SensitivityStudy.h"
#include "SensitivityLogger.h"

namespace po = boost::program_options;

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

int main(int argc, char **argv) {
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
      "output,o", po::value<std::string>(&json_output_path)->required(),
      "path to the sensitivity json output file")(
      "benders,b", po::value<std::string>(&benders_output_path)->required(),
      "path to the benders json output file")(
      "master,m", po::value<std::string>(&last_master_path)->required(),
      "path to the last master mps file")(
      "structure,s", po::value<std::string>(&structure_path)->required(),
      "path to the structure txt file")(
      "log,l", po::value<std::string>(&log_path)->required(),
      "path to the sensitivity log file");

  po::variables_map opts;
  po::store(po::parse_command_line(argc, argv, desc), opts);

  if (opts.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  po::notify(opts);

  auto logger = build_logger(log_path);

  auto sensitivity_input_reader = SensitivityInputReader(
      json_input_path, benders_output_path, last_master_path, structure_path);
  SensitivityInputData input_data = sensitivity_input_reader.get_input_data();

  auto writer = std::make_shared<SensitivityWriter>(json_output_path);
  auto sensitivity_study = SensitivityStudy(input_data, logger, writer);

  try {
    sensitivity_study.launch();
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }
}