#include "OuterLoopInputDataReader.h"

/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something esle necessarily
 * /!\ body could be := Brittany or nothing
 */
OuterLoopPattern::OuterLoopPattern(const std::string &prefix,
                                   const std::string &body)
    : prefix_(prefix), body_(body) {}

/**
 * just do
 * just cat ;)
 */
std::regex OuterLoopPattern::MakeRegex() const {
  auto pattern = "(^" + prefix_ + ").*" + body_;
  return std::regex(pattern);
}