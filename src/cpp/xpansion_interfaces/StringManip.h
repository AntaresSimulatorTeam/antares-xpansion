#pragma once

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
namespace StringManip {

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

inline std::string trim(std::string_view original) {
  auto right =
      std::find_if(original.rbegin(), original.rend(), IsNotSpace()).base();
  auto left = std::find_if(original.begin(), right, IsNotSpace());
  return std::string(left, right);
}

inline std::vector<std::string> split(std::string_view original,
                                      char delimiter = ' ') {
  // TODO C++20 https://en.cppreference.com/w/cpp/ranges/split_view
  std::vector<std::string> strings;
  auto left = original.begin();
  for (auto it = left; it != original.end(); ++it) {
    if (*it == delimiter) {
      strings.emplace_back(&*left, it - left);
      left = it + 1;
    }
  }
  if (left != original.end())
    strings.emplace_back(&*left, original.end() - left);
  return strings;
}
inline std::vector<std::string> split(std::string_view original,
                                      std::string_view delimiter) {
  size_t pos = 0;
  auto copy_original = trim(original);
  std::vector<std::string> result;
  while ((pos = copy_original.find(delimiter)) != std::string::npos) {
    result.push_back(copy_original.substr(0, pos));
    copy_original.erase(0, pos + delimiter.length());
  }
  if (!copy_original.empty()) {
    result.push_back(copy_original);
  }
  return result;
}

class StringUtils {
 public:
  static std::string ToLowercase(std::string_view s) {
    std::string result;
    std::transform(s.cbegin(), s.cend(), std::back_inserter(result),
                   [](char const& c) { return std::tolower(c); });
    return result;
  }
  static bool contains(std::string const& v1, std::string const& v2) {
    return v1.find(v2) != std::string::npos;
  }
};
}  // namespace StringManip