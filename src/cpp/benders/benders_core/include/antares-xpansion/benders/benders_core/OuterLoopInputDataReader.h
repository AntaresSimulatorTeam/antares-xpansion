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

class OuterLoopInputFileError
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};
class OuterLoopInputFileIsEmpty
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopInputFileNoPatternFound
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopInputPatternsShouldBeArray
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopCouldNotReadAreaField
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

class OuterLoopCouldNotReadCriterionField
    : public LogUtils::XpansionError<std::runtime_error> {
  using LogUtils::XpansionError<std::runtime_error>::XpansionError;
};

/// @brief lovely class
class OuterLoopPattern {
 public:
  explicit OuterLoopPattern(std::string prefix, std::string body);
  OuterLoopPattern() = default;
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
class OuterLoopSingleInputData {
 public:
  OuterLoopSingleInputData() = default;
  /// @brief constructor
  /// @param prefix the prefix in the variable's name
  /// @param body any string that could be in the variable's name
  /// @param criterion the criterion that should be satisfied
  OuterLoopSingleInputData(const std::string &prefix, const std::string &body,
                           double criterion);

  [[nodiscard]] OuterLoopPattern Pattern() const;
  [[nodiscard]] double Criterion() const;
  void SetCriterion(double criterion);
  void ResetPattern(const std::string &prefix, const std::string &body);
 private:
  OuterLoopPattern outer_loop_pattern_;
  double criterion_ = 0;
};

/// @brief this class contains all data read from user input file
class OuterLoopInputData {
 public:
  OuterLoopInputData() = default;

  [[nodiscard]] const std::vector<OuterLoopSingleInputData> &OuterLoopData()
      const;
  [[nodiscard]] std::vector<std::string> PatternBodies() const;
  [[nodiscard]] std::string PatternsPrefix() const;

  void SetStoppingThreshold(double outer_loop_stopping_threshold);
  [[nodiscard]] double StoppingThreshold() const;
  void SetCriterionCountThreshold(double criterion_count_threshold);
  [[nodiscard]] double CriterionCountThreshold() const;
  void AddSingleData(const OuterLoopSingleInputData &data);

 private:
  double outer_loop_stopping_threshold_ = 1e-4;
  std::vector<OuterLoopSingleInputData> outer_loop_data_;
  double criterion_count_threshold_ = 1;
};

/// @brief Abstract /***
class IOuterLoopInputDataReader {
 public:
  virtual OuterLoopInputData Read(const std::filesystem::path &input_file) = 0;
  virtual ~IOuterLoopInputDataReader() = default;
};

class OuterLoopInputFromYaml : public IOuterLoopInputDataReader {
 public:
  OuterLoopInputFromYaml() = default;
  OuterLoopInputData Read(const std::filesystem::path &input_file) override;

 private:
  OuterLoopInputData outerLoopInputData_;
};

}  // namespace Benders::Criterion