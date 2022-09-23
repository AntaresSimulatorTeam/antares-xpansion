#ifndef ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONMAIN_H
#define ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONMAIN_H

#include <boost/program_options.hpp>
#include <filesystem>

class ProblemGenerationExeOptions {
 private:
  std::filesystem::path root_;
  std::string master_formulation_;
  std::string additional_constraintFilename_l_;
  boost::program_options::options_description desc_ = "Allowed options";

 public:
  ProblemGenerationExeOptions();
  ~ProblemGenerationExeOptions() = default;
  std::filesystem::path root() const { return root_; }
  std::string master_formulation() const { return master_formulation_; }
  std::string additional_constraintFilename_l() const {
    return additional_constraintFilename_l_;
  }
  void parse(int argc, char** argv);

  template <class T>
  void AddOption(std::string& option_name, bool required, const T* t,
                 const char short_char, std::string& description) {
    auto param = option_name + "," + short_char;
    if (required) {
      desc_.add_options(param, boost::program_options::value<T>(t)->required(),
                        description);
    } else {
      desc_.add_options(param, boost::program_options::value<T>(t),
                        description);
    }
  }
};
int ProblemGenerationMain(int argc, char** argv);
#endif  // ANTARES_XPANSION_SRC_CPP_LPNAMER_MAIN_INCLUDE_PROBLEMGENERATIONMAIN_H