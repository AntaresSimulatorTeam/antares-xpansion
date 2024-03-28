#include "OuterLoopInputDataReader.h"

OuterLoopPattern::OuterLoopPattern(const std::string &prefix,
                                   const std::string &body)
    : prefix_(prefix), body_(body) {}

std::regex OuterLoopPattern::MakeRegex() const {
  auto pattern = "^" + prefix_ + ".*" + body_;
  return std::regex(pattern);
}