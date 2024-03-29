#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace Outerloop {
/// @brief lovely class
class OuterLoopPattern {
 public:
  explicit OuterLoopPattern(const std::string &prefix, const std::string &body);
  std::regex MakeRegex() const;

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

  OuterLoopPattern Pattern() const { return outer_loop_pattern_; }
  double Criterion() const { return criterion_; }

 private:
  OuterLoopPattern outer_loop_pattern_;
  double criterion_;
};

/// @brief this class contains all data read from user input file
class OuterLoopInputData {
 public:
  OuterLoopInputData() = default;

  std::vector<OuterLoopSingleInputData> OuterLoopData() const {
    return outer_loop_data_;
  }

  double StoppingThreshold() const { return outer_loop_stopping_threshold_; }

 private:
  double outer_loop_stopping_threshold_;
  std::vector<OuterLoopSingleInputData> outer_loop_data_;
};

/// @brief Abstract /***
class IOuterLoopInputDataReader {
 public:
  virtual OuterLoopInputData Read(const std::filesystem::path &input_file) = 0;
};

// class OuterLoopInputFromJson {
//   OuterLoopInputFromJson(std)
// };

}  // namespace Outerloop