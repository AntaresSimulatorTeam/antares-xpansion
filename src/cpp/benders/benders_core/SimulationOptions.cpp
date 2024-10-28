#include "antares-xpansion/benders/benders_core/SimulationOptions.h"

#include <json/json.h>

#include <filesystem>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
Json::Value SimulationOptions::get_value_from_json(
    const std::filesystem::path &file_name) {
  Json::Value _input;
  std::ifstream input_file_l(file_name, std::ifstream::binary);
  Json::CharReaderBuilder builder_l;
  std::string errs;
  if (!parseFromStream(builder_l, input_file_l, &_input, &errs)) {
    using namespace std::string_literals;
    auto message = LOGLOCATION + "Invalid options file: "s + file_name.string()
        + "\n" + errs;
    throw InvalidOptionFileException(message);
  }
  return _input;
}
/*!
 *  \brief Constructor of Benders Options
 *
 */
SimulationOptions::SimulationOptions()
    :
#define BENDERS_OPTIONS_MACRO(name__, type__, default__, \
                              deserialization_method__)  \
  name__(default__),
#include "antares-xpansion/benders/benders_core/SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
      _weights() {
}

/*!
 *  \brief Constructor of Benders Options
 *  \param options_filename file that contains options
 */
SimulationOptions::SimulationOptions(
    const std::filesystem::path &options_filename)
    : SimulationOptions() {
  read(options_filename);
}

/*!
 *  \brief Write default options in "options_default" txt file
 */
void SimulationOptions::write_default() const {
  std::ofstream file("options_default.txt");
  print(file);
  file.close();
}

/*!
 *  \brief Read Benders options from file path
 *
 *  \param file_name : path to options txt file
 */
void SimulationOptions::read(const std::filesystem::path &file_name) {
  const auto options_values = get_value_from_json(file_name);
  for (const auto &var_name : options_values.getMemberNames()) {
#define BENDERS_OPTIONS_MACRO(var__, type__, default__, \
                              deserialization_method__) \
  if (#var__ == var_name)                               \
    var__ = options_values[var_name].deserialization_method__;
#include "antares-xpansion/benders/benders_core/SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
  }
  set_weights();
}

void SimulationOptions::set_weights() {
  if (SLAVE_WEIGHT != SUBPROBLEM_WEIGHT_UNIFORM_CST_STR &&
      SLAVE_WEIGHT != SUBPROBLEM_WEIGHT_CST_STR) {
    std::string line;
    auto filename(std::filesystem::path(INPUTROOT) / SLAVE_WEIGHT);
    std::ifstream file(filename);

    if (!file) {
      std::cout << "Cannot open file " << filename << std::endl;
    }
    double weights_sum = -1;
    while (std::getline(file, line)) {
      std::stringstream buffer(line);
      std::string problem_name;

      buffer >> problem_name;
      if (problem_name == WEIGHT_SUM_CST_STR) {
        buffer >> weights_sum;
      } else {
        buffer >> _weights[problem_name];
      }
    }

    if (weights_sum == -1) {
      std::cerr
          << LOGLOCATION
          << "ERROR : Invalid weight file format : Key WEIGHT_SUM not found."
          << std::endl;
      std::exit(1);
    } else {
      for (const auto &kvp : _weights) {
        _weights[kvp.first] /= weights_sum;
      }
    }
  }
}

/*!
 *  \brief Print all Benders options
 *
 *  \param stream : output stream
 */
void SimulationOptions::print(std::ostream &stream) const {
#define BENDERS_OPTIONS_MACRO(name__, type__, default__, \
                              deserialization_method__)  \
  stream << std::setw(30) << #name__ << std::setw(50)<<std::boolalpha << name__ << std::endl;
#include "antares-xpansion/benders/benders_core/SimulationOptions.hxx"
#undef BENDERS_OPTIONS_MACRO
  stream << std::endl;
}

BaseOptions SimulationOptions::get_base_options() const {
  BaseOptions result;

  result.LOG_LEVEL = LOG_LEVEL;

  result.SLAVE_WEIGHT_VALUE = SLAVE_WEIGHT_VALUE;

  result.OUTPUTROOT = OUTPUTROOT;
  result.LAST_ITERATION_JSON_FILE = LAST_ITERATION_JSON_FILE;
  result.SLAVE_WEIGHT = SLAVE_WEIGHT;
  result.MASTER_NAME = MASTER_NAME;
  result.STRUCTURE_FILE = STRUCTURE_FILE;
  result.INPUTROOT = INPUTROOT;
  result.SOLVER_NAME = SOLVER_NAME;
  result.weights = _weights;
  result.RESUME = RESUME;
  result.AREA_FILE = AREA_FILE;

  return result;
}

BendersBaseOptions SimulationOptions::get_benders_options() const {
  BendersBaseOptions result(get_base_options());

  result.MAX_ITERATIONS = MAX_ITERATIONS;

  result.ABSOLUTE_GAP = ABSOLUTE_GAP;
  result.RELATIVE_GAP = RELATIVE_GAP;
  result.RELAXED_GAP = RELAXED_GAP;
  result.TIME_LIMIT = TIME_LIMIT;
  result.SEPARATION_PARAM = SEPARATION_PARAM;

  if (MASTER_FORMULATION == "integer") {
    result.MASTER_FORMULATION = MasterFormulation::INTEGER;
  } else if (MASTER_FORMULATION == "relaxed") {
    result.MASTER_FORMULATION = MasterFormulation::RELAXED;
  } else {
    std::cerr << LOGLOCATION << "Invalid value " << MASTER_FORMULATION
              << " for option master" << std::endl;
    std::exit(1);
  }

  result.AGGREGATION = AGGREGATION;
  result.TRACE = TRACE;
  result.BOUND_ALPHA = BOUND_ALPHA;

  result.CSV_NAME = CSV_NAME;
  result.LAST_MASTER_MPS = LAST_MASTER_MPS;
  result.LAST_MASTER_BASIS = LAST_MASTER_BASIS;
  result.BATCH_SIZE = BATCH_SIZE;
  result.EXTERNAL_LOOP_OPTIONS = GetExternalLoopOptions();
  return result;
}
SimulationOptions::InvalidOptionFileException::InvalidOptionFileException(
    const std::string &arg)
    : runtime_error(arg) {}


ExternalLoopOptions SimulationOptions::GetExternalLoopOptions() const {
  return {DO_OUTER_LOOP, OUTER_LOOP_OPTION_FILE};
}