#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "LoggerUtils.h"
#include "yaml-cpp/yaml.h"

namespace AdequacyCriterionSpace {

class AdequacyCriterionInputFileError
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};
class AdequacyCriterionInputFileIsEmpty
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class AdequacyCriterionInputFileNoPatternFound
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class AdequacyCriterionInputPatternsShouldBeArray
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class AdequacyCriterionCouldNotReadAreaField
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class AdequacyCriterionCouldNotReadCriterionField
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

/// @brief lovely class
class AdequacyCriterionPattern {
 public:
  explicit AdequacyCriterionPattern(std::string prefix, std::string body);
  AdequacyCriterionPattern() = default;
  [[nodiscard]] std::regex MakeRegex() const;
  [[nodiscard]] const std::string &GetPrefix() const;
  void SetPrefix(const std::string &prefix);
  [[nodiscard]] const std::string &GetBody() const;
  void SetBody(const std::string &body);

 private:
  std::string prefix_;
  std::string body_;
};

/// @brief holds the pattern and the criterion of the outer loop
class AdequacyCriterionSingleInputData {
 public:
  AdequacyCriterionSingleInputData() = default;
  /// @brief constructor
  /// @param prefix the prefix in the variable's name
  /// @param body any string that could be in the variable's name
  /// @param criterion the criterion that should be satisfied
  AdequacyCriterionSingleInputData(const std::string &prefix,
                                   const std::string &body, double criterion);

  [[nodiscard]] AdequacyCriterionPattern Pattern() const;
  [[nodiscard]] double Criterion() const;
  void SetCriterion(double criterion);
  void ResetPattern(const std::string &prefix, const std::string &body);
 private:
  AdequacyCriterionPattern adequacy_criterion_pattern_;
  double criterion_ = 0;
};

/// @brief this class contains all data read from user input file
class AdequacyCriterionInputData {
 public:
  AdequacyCriterionInputData() = default;

  [[nodiscard]] std::vector<AdequacyCriterionSingleInputData>
  AdequacyCriterionData() const;
  [[nodiscard]] std::vector<std::string> PatternBodies() const;
  [[nodiscard]] std::string PatternsPrefix() const;

  void SetStoppingThreshold(double adequacy_criterion_stopping_threshold);
  [[nodiscard]] double StoppingThreshold() const;
  void SetCriterionCountThreshold(double criterion_count_threshold);
  [[nodiscard]] double CriterionCountThreshold() const;
  void AddSingleData(const AdequacyCriterionSingleInputData &data);

 private:
  double adequacy_criterion_stopping_threshold_ = 1e-4;
  std::vector<AdequacyCriterionSingleInputData> adequacy_criterion_data_;
  double criterion_count_threshold_ = 1;
};

/// @brief Abstract /***
class IAdequacyCriterionInputDataReader {
 public:
  virtual AdequacyCriterionInputData Read(
      const std::filesystem::path &input_file) = 0;
};

class AdequacyCriterionInputFromYaml
    : public IAdequacyCriterionInputDataReader {
 public:
  AdequacyCriterionInputFromYaml() = default;
  AdequacyCriterionInputData Read(
      const std::filesystem::path &input_file) override;

 private:
  AdequacyCriterionInputData outerLoopInputData_;
};

}  // namespace AdequacyCriterionSpace