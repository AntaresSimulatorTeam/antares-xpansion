#include "WeightsFileReader.h"

#include <fstream>
bool WeightsFileReader::CheckWeightsFile() const {
  std::ifstream file_reader(weights_file_path_);
  if (!file_reader) {
    (*logger_)(ProblemGenerationLog::LOGLEVEL::ERROR)
        << "Could not open weights file: " << weights_file_path_ << std::endl;
    return false;
  }
  return true;
}