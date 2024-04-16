#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

#include "LoggerUtils.h"
#include "common.h"

namespace Outerloop {

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
  explicit OuterLoopPattern(const std::string &prefix, const std::string &body);
  [[nodiscard]] std::regex MakeRegex() const;
  [[nodiscard]] const std::string &GetPrefix() const;
  [[nodiscard]] const std::string &GetBody() const;

 private:
  std::string prefix_;
  std::string body_;

};

/// @brief holds the pattern and the criterion of the outer loop
class OuterLoopSingleInputData {
 public:
  /// @brief constructor
  /// @param prefix the prefix in the variable's name
  /// @param body any string that could be in the variable's name
  /// @param criterion the criterion that should be satisfied
  OuterLoopSingleInputData(const std::string &prefix, const std::string &body,
                           double criterion);

  OuterLoopPattern Pattern() const;
  double Criterion() const;

 private:
  OuterLoopPattern outer_loop_pattern_;
  double criterion_;
};

/// @brief this class contains all data read from user input file
class OuterLoopInputData {
 public:
  OuterLoopInputData() = default;

  std::vector<OuterLoopSingleInputData> OuterLoopData() const;

  void SetStoppingThreshold(double outer_loop_stopping_threshold);
  [[nodiscard]] double StoppingThreshold() const;
  void SetCriterionTolerance(double criterion_tolerance);
  [[nodiscard]] double CriterionTolerance() const;
  void SetCriterionCountThreshold(double criterion_count_threshold);
  [[nodiscard]] double CriterionCountThreshold() const;
  void AddSingleData(const OuterLoopSingleInputData &data);

 private:
  double outer_loop_stopping_threshold_ = 1e-4;
  std::vector<OuterLoopSingleInputData> outer_loop_data_;
  double criterion_tolerance_ = 1e-1;
  double criterion_count_threshold_ = 1e-1;
};

/// @brief Abstract /***
class IOuterLoopInputDataReader {
 public:
  virtual OuterLoopInputData Read(const std::filesystem::path &input_file) = 0;
};

class OuterLoopInputFromYaml : public IOuterLoopInputDataReader {
 public:
  OuterLoopInputFromYaml() = default;
  OuterLoopInputData Read(const std::filesystem::path &input_file) override;

 private:
  void Decode(const Json::Value &json_content);
  void DecodePatterns(const Json::Value &patterns);
  void DecodePattern(const Json::Value &pattern);
  OuterLoopInputData outerLoopInputData_;
};

}  // namespace Outerloop