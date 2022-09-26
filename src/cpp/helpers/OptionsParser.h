#ifndef ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H

#include <boost/program_options.hpp>
class OptionsParser {
 private:
  boost::program_options::options_description desc_ =
      boost::program_options::options_description("Allowed options");

 public:
  OptionsParser() = default;
  virtual ~OptionsParser() = default;

  void parse(int argc, char** argv);

  //   template <class T>
  //   void AddOption(std::string& option_name, bool required, const T* t,
  //                  const char short_char, std::string& description) {
  //     auto param = option_name + "," + short_char;
  //     if (required) {
  //       desc_.add_options()(
  //           param, boost::program_options::value<T>(t)->required(),
  //           description);
  //     } else {
  //       desc_.add_options()(param, boost::program_options::value<T>(t),
  //                           description);
  //     }
  //   }
  boost::program_options::options_description_easy_init AddOptions() {
    return desc_.add_options();
  }
};
#endif  // ANTARES_XPANSION_SRC_CPP_HELPERS_OPTIONSPARSER_H
