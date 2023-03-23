#pragma once
#include <stdexcept>
#include <string>

inline std::string LogLocationToStr(int line, const char* file,
                                    const char* func) {
  return std::string("This is line ") + std::to_string(line) + " of file " +
         file + " (function " + func + ")\n";
}
#define LOGLOCATION LogLocationToStr(__LINE__, __FILE__, __func__)

template <typename T>
class XpansionError : public T {
 public:
  explicit XpansionError(const std::string& err_message,
                         const std::string& log_location)
      : T(log_location + err_message), err_message_(err_message) {}

 public:
  std::string ErrorMessage() const { return err_message_; }

 private:
  const std::string err_message_;
};