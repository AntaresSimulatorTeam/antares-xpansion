#include "OuterLoopInputDataReader.h"

/**
<<<<<<< HEAD
 * prefix could be := PositiveUnsuppliedEnergy:: or something else necessarily
=======
 * prefix could be := PositiveUnsuppliedEnergy:: or something esle necessarily
>>>>>>> 9ab05d5657d66ccead3f2f821cda7908ac5a7030
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