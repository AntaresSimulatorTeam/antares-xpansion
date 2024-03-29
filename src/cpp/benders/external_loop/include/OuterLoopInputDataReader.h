#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

namespace Outerloop {
/// @brief lovely class links techs and BA
class OuterLoopPattern {
 public:
  explicit OuterLoopPattern(const std::string &prefix, const std::string &body);
  std::regex MakeRegex() const;

 private:
  std::string prefix_;
  std::string body_;
};

/// @brief Abstract /***
class IOuterLoopInputDataReader {
 public:
  virtual void Read(const std::filesystem::path &input_file) = 0;

 private:
  /*
   double = criterion; OuterLoopPattern :={variable identity, area} // YOU NAMED
   IT ;)
   *
   */
  std::tuple<std::vector<double>, std::vector<OuterLoopPattern>> data;
};

// class OuterLoopInputFromJson {
//   OuterLoopInputFromJson(std)
// };

}  // namespace Outerloop