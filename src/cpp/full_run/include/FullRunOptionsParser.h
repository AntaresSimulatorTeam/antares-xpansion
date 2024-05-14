#ifndef ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#include <stdexcept>

#include "BendersFactory.h"
#include "ProblemGenerationExeOptions.h"

class FullRunOptionsParser : public ProblemGenerationExeOptions {
 public:
  FullRunOptionsParser();
  ~FullRunOptionsParser() override = default;
  void Parse(unsigned int argc, const char* const* argv) override;

  [[nodiscard]] std::filesystem::path BendersOptionsFile() const {
    return benders_options_file_;
  }
  [[nodiscard]] std::filesystem::path SolutionFile() const { return solutionFile_; }

 private:
  std::filesystem::path benders_options_file_;
  std::filesystem::path solutionFile_;
};

#endif  // ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
