#include "WeightsFileReader.h"

#include <fstream>
#include <sstream>

#include "common_lpnamer.h"

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
  weights_list_.clear();
  null_weights = true;
  size_t nb_values = 0;
  int idx = 0;
  std::string line;
  while (std::getline(file_reader, line)) {
    auto weight = GetWeightFromLine(line, idx);
    nb_values += 1;
    CheckValue(weight, idx);
    weights_list_.push_back(weight);
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
  file_reader.close();
  return true;
}

double WeightsFileReader::GetWeightFromLine(const std::string &line,
                                            int idx) const {
  double weight;
  try {
    weight = std::stod(common_lpnamer::trim(line));
  } catch (...) {
    std::ostringstream msg;
    msg << "Line " << idx << " in file " << weights_file_path_.string()
        << "is not a single non-negative value" << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL) << msg.str();
    throw InvalidYearsWeightValue(msg.str());
  }
  return weight;
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