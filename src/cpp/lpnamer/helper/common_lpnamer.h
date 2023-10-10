#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace common_lpnamer {
const std::string MPS_TXT{"mps.txt"};

template <std::ctype_base::mask mask>
class IsNot {
  std::locale myLocale;  // To ensure lifetime of facet...
  std::ctype<char> const* myCType;

 public:
  explicit IsNot(std::locale const& l = std::locale())
      : myLocale(l), myCType(&std::use_facet<std::ctype<char> >(l)) {}
  bool operator()(char ch) const { return !myCType->is(mask, ch); }
};

using IsNotSpace = IsNot<std::ctype_base::space>;

inline std::string trim(std::string const& original) {
  auto right =
      std::find_if(original.rbegin(), original.rend(), IsNotSpace()).base();
  auto left = std::find_if(original.begin(), right, IsNotSpace());
  return std::string(left, right);
}

inline std::vector<std::string> split(const std::string& original,
                                      char delimiter = ' ') {
  std::vector<std::string> strings;
  std::istringstream f(original);
  std::string s;
  while (std::getline(f, s, delimiter)) {
    strings.push_back(s);
  }
  return strings;
}

inline FILE* OpenLogPtr(const std::filesystem::path& log_file) {
  FILE* log_file_ptr = NULL;
  if (log_file.empty()) {
    return log_file_ptr;
  }
#ifdef __linux__
  if ((log_file_ptr = fopen(log_file.string().c_str(), "a+")) == nullptr)
#elif _WIN32
  if ((log_file_ptr = _fsopen(log_file.string().c_str(), "a+", _SH_DENYNO)) ==
      nullptr)
#endif
  {
    std::cerr << "Invalid log file name passed as parameter: " << log_file
              << std::endl;
  }
  return log_file_ptr;
}
}  // namespace common_lpnamer
