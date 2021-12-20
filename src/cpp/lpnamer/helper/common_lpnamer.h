#pragma once

#include <list>
#include <map>
#include <set>
#include <string>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
const std::string PATH_SEPARATOR{"\\"};
#else
const std::string PATH_SEPARATOR{"/"};
#endif

inline bool contains(std::string const &v1, std::string const &v2) {
  return v1.find(v2) != std::string::npos;
}
inline int to_int(std::string const &value) {
  std::stringstream buffer;
  buffer << value;
  int result;
  buffer >> result;
  return result;
}
