#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "antares-xpansion/xpansion_interfaces/LoggerUtils.h"
#include "yaml-cpp/yaml.h"

namespace Benders::Criterion {
static constexpr const char *const PositiveUnsuppliedEnergy =
    "PositiveUnsuppliedEnergy::";

class CriterionInputFileError
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};
class CriterionInputFileIsEmpty
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class CriterionInputFileNoPatternFound
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class CriterionInputPatternsShouldBeArray
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class CouldNotReadAreaField
  : public LogUtils::XpansionError<std::runtime_error> {
 using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class CouldNotReadCriterionField
  : public LogUtils::XpansionError<std::runtime_error> {
 using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

/// @brief lovely class
class CriterionPattern {
 public:
  explicit CriterionPattern(std::string prefix, std::string body);
  CriterionPattern() = default;
  [[nodiscard]] std::regex MakeRegex() const;
  [[nodiscard]] const std::string &GetPrefix() const;
  void SetPrefix(const std::string &prefix);
  [[nodiscard]] const std::string &GetBody() const;
  void SetBody(const std::string &body);

 private:
  std::string prefix_;
  std::string body_;

};

/// @brief holds the pattern and the criterion
class CriterionSingleInputData {
 public:
  CriterionSingleInputData() = default;
  /// @brief constructor
  /// @param prefix the prefix in the variable's name
  /// @param body any string that could be in the variable's name
  /// @param criterion the criterion that should be satisfied
  CriterionSingleInputData(const std::string &prefix, const std::string &body,
                           double criterion);

  [[nodiscard]] CriterionPattern Pattern() const;
  [[nodiscard]] double Criterion() const;
  void SetCriterion(double criterion);
  void ResetPattern(const std::string &prefix, const std::string &body);
 private:
  CriterionPattern pattern_;
  double criterion_ = 0;
};

/// @brief this class contains all data read from user input file
class CriterionInputData {
 public:
  CriterionInputData() = default;

  [[nodiscard]] const std::vector<CriterionSingleInputData> &Criteria()
  const;

  [[nodiscard]] std::vector<std::string> PatternBodies() const;
  [[nodiscard]] std::string PatternsPrefix() const;

  void SetCriterionCountThreshold(double count_threshold);
  [[nodiscard]] double CriterionCountThreshold() const;
  void AddSingleData(const CriterionSingleInputData &data);

 private:
  std::vector<CriterionSingleInputData> criterion_vector_;
  double count_threshold_ = 1;
};

/// @brief this class contains all data read from user input file
class OuterLoopCriterionInputData : public CriterionInputData {
 public:
  OuterLoopCriterionInputData() = default;
  [[nodiscard]] double StoppingThreshold() const;
  void setStoppingThreshold(double stoppingThreshold);

 private:
  double stopping_threshold_ = 1e-4;
};

/// @brief Abstract
class ICriterionInputDataReader {
 public:
  virtual OuterLoopCriterionInputData Read(
      const std::filesystem::path &input_file) = 0;
  virtual ~ICriterionInputDataReader() = default;
};

class CriterionInputFromYaml : public ICriterionInputDataReader {
 public:
  CriterionInputFromYaml() = default;
  OuterLoopCriterionInputData Read(
      const std::filesystem::path &input_file) override;

 private:
  CriterionInputData criterion_input_data_;
};

}  // namespace Benders::Criterion