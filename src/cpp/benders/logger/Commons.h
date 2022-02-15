#ifndef ANTARESXPANSION_COMMONS_H
#define ANTARESXPANSION_COMMONS_H

namespace xpansion {
namespace logger {
namespace commons {

inline double convert_in_million_euros(double val) { return val / 1e6; }

inline std::string create_str_million_euros(double val) {
  std::stringstream result;
  result << std::fixed << std::setprecision(2) << convert_in_million_euros(val);
  return result.str();
}

inline std::string create_str_euros(double val) {
  std::stringstream result;
  result << std::scientific << std::setprecision(5) << val;
  return result.str();
}
}  // namespace commons
}  // namespace logger
}  // namespace xpansion
#endif  // ANTARESXPANSION_COMMONS_H
