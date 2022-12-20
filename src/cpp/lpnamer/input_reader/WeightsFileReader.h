#ifndef SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#define SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#include <filesystem>
#include <set>
#include <stdexcept>

#include "ProblemGenerationLogger.h"

class WeightsFileReader {
 private:
  std::filesystem::path weights_file_path_;
  std::set<int> active_years_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  std::ifstream file_reader_;
  int CountValues() const;
  bool AreAllWeightsNull() const;

 public:
  explicit WeightsFileReader(
      const std::filesystem::path& weights_file_path,
      const std::set<int>& active_years,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : weights_file_path_(weights_file_path),
        active_years_(active_years),
        logger_(logger) {}

  class WeightsFileOpenError : public std::runtime_error {
   public:
    WeightsFileOpenError(const std::string& msg) : std::runtime_error(msg) {}
  };

  /// @brief checks that the yearly-weights file exists and has correct format:
  // column of non-negative weights
  // sum of weights is positive
  // nb_weight equal nb_active_yearse
  /// @return true if it's ok
  bool CheckWeightsFile();
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H