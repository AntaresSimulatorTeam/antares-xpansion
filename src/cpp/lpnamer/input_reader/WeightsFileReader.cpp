#include "WeightsFileReader.h"

#include <fstream>
#include <sstream>
bool WeightsFileReader::CheckWeightsFile() {
  if (!file_reader_.is_open()) {
    file_reader_.open(weights_file_path_);
  }
  if (!file_reader_) {
    std::ostringstream msg;
    msg << "Could not open weights file: " << weights_file_path_.string()
        << std::endl;
    (*logger_)(ProblemGenerationLog::LOGLEVEL::ERROR) << msg.str();
    throw WeightsFileOpenError(msg.str());
  }

  auto null_weights = true;
  int nb_values = 0;
  return true;
}

int WeightsFileReader::CountValues() const { return 0; }
bool WeightsFileReader::AreAllWeightsNull() const { return true; }