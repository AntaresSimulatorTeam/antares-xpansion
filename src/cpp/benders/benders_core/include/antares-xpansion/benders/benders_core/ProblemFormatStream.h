#pragma once

#include <fmt/format.h>

#include <ostream>

#include "ProblemFormat.h"

inline std::ostream &operator<<(std::ostream &stream,
                                ProblemsFormat const &rhs) {
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

template <>
struct fmt::formatter<ProblemsFormat> : formatter<string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(ProblemsFormat problems_format,
              format_context &ctx) const -> format_context::iterator;
};
