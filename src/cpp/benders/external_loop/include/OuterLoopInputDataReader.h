#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <tuple>
#include <vector>
/// @brief lovely class links techs and BA
class OuterLoopPattern {
 public:
  explicit OuterLoopPattern(const std::string &prefix, const std::string &body);
  std::regex MakeRegex() const;

 private:
  std::string prefix_;
  std::string body_;
};

/// @brief Abstract /*** eaaaaaaaaaaaaasy yaaaaaaaaaall
class IOuterLoopInputDataReader {
 public:
  virtual void Read(const std::filesystem::path &input_file) = 0;

 private:
  /*
   double = criterion; regex :={variable identity, area} // YOU NAMED IT ;)
   *
   */
  std::tuple<double, std::vector<std::regex>> data;
};

//*** = RTE