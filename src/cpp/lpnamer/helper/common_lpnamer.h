#pragma once

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace common_lpnamer {
template <std::ctype_base::mask mask>
class IsNot {
  std::locale myLocale;  // To ensure lifetime of facet...
  std::ctype<char> const* myCType;

 public:
  IsNot(std::locale const& l = std::locale())
      : myLocale(l), myCType(&std::use_facet<std::ctype<char> >(l)) {}
  bool operator()(char ch) const { return !myCType->is(mask, ch); }
};

typedef IsNot<std::ctype_base::space> IsNotSpace;

inline std::string trim(std::string const& original) {
  std::string::const_iterator right =
      std::find_if(original.rbegin(), original.rend(), IsNotSpace()).base();
  std::string::const_iterator left =
      std::find_if(original.begin(), right, IsNotSpace());
  return std::string(left, right);
}

inline std::vector<std::string> split(const std::string original,
                                      char delimiter = ' ') {
  std::vector<std::string> strings;
  std::istringstream f(original);
  std::string s;
  while (std::getline(f, s, delimiter)) {
    strings.push_back(s);
  }
  return strings;
}
}  // namespace common_lpnamer
