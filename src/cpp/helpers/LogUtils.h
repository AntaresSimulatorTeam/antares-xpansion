#pragma once
#include <string>
inline std::string LogLocationToStr(int line, const char* file,
                                    const char* func) {
  return std::string("This is line ") + std::to_string(line) + " of file " +
         file + " (function " + func + ")\n";
}
#define LOGLOCATION LogLocationToStr(__LINE__, __FILE__, __func__)