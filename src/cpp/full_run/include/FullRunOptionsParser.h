#ifndef ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#include <stdexcept>

#include "ProblemGenerationExeOptions.h"

class FullRunOptionsParser : public ProblemGenerationExeOptions {
 public:
  FullRunOptionsParser();
  virtual ~FullRunOptionsParser() = default;
  void Parse(unsigned int argc, const char* const* argv) override;
  enum class METHOD { SEQUENTIAL, MPI, MERGEMPS };
  class FullRunOptionInvalidMethod : public std::runtime_error {
   public:
    explicit FullRunOptionInvalidMethod(const std::string& method)
        : std::runtime_error(
              std::string("Eror in FullRunOptionParser, method: ") + method +
              " is not supported! Please choose one of: " +
              FullRunOptionsParser::ListAvailalbleMethods()) {}
  };
  METHOD Method() const { return method_; };
  std::filesystem::path BendersOptionsFile() const {
    return benders_options_file_;
  }
  std::filesystem::path SolutionFile() const { return solutionFile_; }
  static inline std::string ListAvailalbleMethods() {
    return "sequential,\n mpibenders,\n mergeMPS\n";
  }

 private:
  std::string method_str_;
  FullRunOptionsParser::METHOD method_;
  std::filesystem::path benders_options_file_;
  std::filesystem::path solutionFile_;
  void SetMethod();
};

#endif  // ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
