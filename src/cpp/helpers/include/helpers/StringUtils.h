#ifndef ANTARESXPANSION_STRINGUTILS_H
#define ANTARESXPANSION_STRINGUTILS_H

#include <string>

class StringUtils {
 public:
  static std::string ToLowercase(const std::string& s);
  static inline bool contains(std::string const& v1, std::string const& v2) {
    return v1.find(v2) != std::string::npos;
  }
};

#endif  // ANTARESXPANSION_STRINGUTILS_H
