#pragma once

#include <stdexcept>
#include <string>
enum class ProblemsFormat { MPS_FILE, SAVED_FILE };

inline ProblemsFormat problemsFormatFromString(const std::string &str) {
  if (str == "MPS") {
    return ProblemsFormat::MPS_FILE;
  } else if (str == "SAVED") {
    return ProblemsFormat::SAVED_FILE;
  } else {
    throw std::runtime_error("Unknown ProblemsFormat: " + str);
  }
}

inline std::ostream &operator<<(std::ostream &stream, ProblemsFormat const &rhs) {
  switch (rhs) {
    case ProblemsFormat::MPS_FILE:
      stream << "MPS_FILE";
      break;
    case ProblemsFormat::SAVED_FILE:
      stream << "SAVED_FILE";
      break;
    default:
      stream << "Unknown";
  }
  return stream;
}