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