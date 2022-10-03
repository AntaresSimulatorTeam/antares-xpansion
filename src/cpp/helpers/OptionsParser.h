#ifndef ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H

#include <boost/program_options.hpp>
#include <string>
#include <stdexcept>

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
  void Parse(unsigned int argc, const char* const* argv) const;
  class NullArgumentsValues : public std::runtime_error {
   public:
    explicit NullArgumentsValues(const std::string& exe_name)
        : std::runtime_error(std::string("Error while parsing ")+exe_name+" options: null Arguments values!"){};
  };
  class InvalidNumberOfArgumentsPassedToParser : public std::runtime_error {
   public:
    explicit InvalidNumberOfArgumentsPassedToParser(int argc,
                                                    const std::string& exe_name)
        : std::runtime_error(std::string("Error while parsing ") +
                             exe_name +
                             ": invalid number arguments:  " + std::to_string( argc)){};
  };


};
#endif  // ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
