#ifndef ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H

#include <boost/program_options.hpp>
#include <stdexcept>
#include <string>

#include "LogUtils.h"

class OptionsParser {
 private:
  boost::program_options::options_description desc_ =
      boost::program_options::options_description("Allowed options");
  std::string exe_name_ = "";

 protected:
  explicit OptionsParser(const std::string& exe_name) : exe_name_(exe_name){};

 public:
  OptionsParser() = default;
  virtual ~OptionsParser() = default;

  boost::program_options::options_description_easy_init AddOptions() {
    return desc_.add_options();
  }
  virtual void Parse(unsigned int argc, const char* const* argv);
  class NullArgumentsValues
      : public LogUtils::XpansionError<std::runtime_error> {
   public:
    explicit NullArgumentsValues(const std::string& exe_name,
                                 const std::string& log_location)
        : LogUtils::XpansionError<std::runtime_error>(
              "Error while parsing " + exe_name +
                  " options: null Arguments values!",
              log_location){};
  };
  class InvalidNumberOfArgumentsPassedToParser
      : public LogUtils::XpansionError<std::runtime_error> {
   public:
    explicit InvalidNumberOfArgumentsPassedToParser(
        int argc, const std::string& exe_name, const std::string& log_location)
        : LogUtils::XpansionError<std::runtime_error>(
              "Error while parsing " + exe_name +
                  ": invalid number arguments:  " + std::to_string(argc),
              log_location){};
  };
};
#endif  // ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
