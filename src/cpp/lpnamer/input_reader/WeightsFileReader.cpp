#include "WeightsFileReader.h"

#include <fstream>
#include <sstream>

std::string trim(std::string const &original) {
  std::string::const_iterator right =
      std::find_if(original.rbegin(), original.rend(), IsNotSpace()).base();
  std::string::const_iterator left =
      std::find_if(original.begin(), right, IsNotSpace());
  return std::string(left, right);
}
bool WeightsFileReader::CheckWeightsFile() {
  std::ifstream file_reader;
  if (!file_reader.is_open()) {
    file_reader.open(weights_file_path_);
  }
  if (!file_reader) {
    std::ostringstream msg;
    msg << "Could not open weights file: " << weights_file_path_.string()
        << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::ERROR) << msg.str();
    throw WeightsFileOpenError(msg.str());
  }
  CheckFileIsNotEmpty(file_reader);
  null_weights = true;
  int nb_values = 0;
  int idx = 0;
  std::string line;
  while (std::getline(file_reader, line)) {
    auto line_value = GetLineValue(line, idx);
    nb_values += 1;
    CheckValue(line_value, idx);
    idx++;
  }

  if (nb_values != number_of_active_years_) {
    std::ostringstream msg;
    msg << " file " << weights_file_path_.string()
        << " : invalid weight number : " << nb_values << " values / "
        << number_of_active_years_ << " expected" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw InvalidYearsWeightNumber(msg.str());
  }
  if (null_weights) {
    std::ostringstream msg;
    msg << "file " << weights_file_path_.string() << ": all values are null"
        << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw OnlyNullYearsWeightValue(msg.str());
  }
  return true;
}

double WeightsFileReader::GetLineValue(const std::string &line, int idx) const {
  double line_value;
  try {
    line_value = std::stod(trim(line));
  } catch (const std::invalid_argument &e) {
    std::ostringstream msg;
    msg << "Line " << idx << " in file " << weights_file_path_.string()
        << "is not a single non-negative value" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw InvalidYearsWeightValue(msg.str());
  }
  return line_value;
}
void WeightsFileReader::CheckValue(const double value, const int index) {
  if (value > 0) {
    null_weights = false;
  }
  if (value < 0) {
    std::ostringstream msg;
    msg << "Line " << index + 1 << " in file " << weights_file_path_.string()
        << " indicates a negative value" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw InvalidYearsWeightValue(msg.str());
  }
}
bool WeightsFileReader::AreAllWeightsNull() const { return null_weights; }
void WeightsFileReader::CheckFileIsNotEmpty(std::ifstream &file) const {
  if (file.peek() == std::ifstream::traits_type::eof()) {
    std::ostringstream msg;
    msg << "Error! Weights file is empty: " << weights_file_path_.string()
        << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw WeightsFileIsEmtpy(msg.str());
  }
}