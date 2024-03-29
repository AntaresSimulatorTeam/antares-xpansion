#include "OuterLoopInputDataReader.h"

using namespace Outerloop;
/**
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
 * /!\ body could be := 'Brittany' or equivalent or nothing
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

