#ifndef SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#define SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H
#include <filesystem>
#include <stdexcept>
#include <vector>

#include "ProblemGenerationLogger.h"

class WeightsFileReader {
 private:
  std::filesystem::path weights_file_path_;
  int number_of_active_years_;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  bool null_weights = true;
  std::vector<int> weights_list_;

  int CountValues() const;
  bool AreAllWeightsNull() const;
  double GetWeightFromLine(const std::string& line, int idx) const;
  void CheckValue(const double line, int idx);
  void CheckFileIsNotEmpty(std::ifstream& file) const;

 public:
  explicit WeightsFileReader(
      const std::filesystem::path& weights_file_path,
      const int number_of_active_years,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : weights_file_path_(weights_file_path),
        number_of_active_years_(number_of_active_years),
        logger_(logger) {}
  class WeightsFileError : public std::runtime_error {
   public:
    WeightsFileError(const std::string& msg) : std::runtime_error(msg) {}
  };
  class WeightsFileOpenError : public WeightsFileError {
    using WeightsFileError::WeightsFileError;
  };
  class InvalidYearsWeightValue : public WeightsFileError {
    using WeightsFileError::WeightsFileError;
  };
  class InvalidYearsWeightNumber : public WeightsFileError {
    using WeightsFileError::WeightsFileError;
  };
  class OnlyNullYearsWeightValue : public WeightsFileError {
    using WeightsFileError::WeightsFileError;
  };
  class WeightsFileIsEmtpy : public WeightsFileError {
    using WeightsFileError::WeightsFileError;
  };

  /// @brief checks that the yearly-weights file exists and has correct
  /// format:
  // column of non-negative weights
  // sum of weights is positive
  // nb_weight equal nb_active_yearse
  /// @return true if it's ok
  bool CheckWeightsFile();
  std::vector<int> WeightsList() const { return weights_list_; }
};
#endif  // SRC_CPP_LPNAMER_INPUTREADER_WEIGHTSFILEREADER_H