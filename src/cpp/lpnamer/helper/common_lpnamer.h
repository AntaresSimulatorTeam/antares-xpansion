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

inline int to_int(std::string const& value) {
  std::stringstream buffer;
  buffer << value;
  int result;
  buffer >> result;
  return result;
}
