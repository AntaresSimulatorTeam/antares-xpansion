#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <tuple>
#include <vector>

class OuterLoopPattern {
 public:
  explicit OuterLoopPattern(const std::string &prefix, const std::string &body);
  std::regex MakeRegex() const;

 private:
  std::string prefix_;
  std::string body_;
};

class IOuterLoopInputDataReader {
 public:
  virtual void Read(const std::filesystem::path &input_file) = 0;

 private:
  std::tuple<double, std::vector<std::regex>> data;
};