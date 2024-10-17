#ifndef ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#include <stdexcept>

#include "antares-xpansion/benders/factories/BendersFactory.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"

class FullRunOptionsParser : public ProblemGenerationExeOptions {
 public:
  FullRunOptionsParser();
  ~FullRunOptionsParser() override = default;
  void Parse(unsigned int argc, const char* const* argv) override;

  [[nodiscard]] std::filesystem::path BendersOptionsFile() const {
    return benders_options_file_;
  }
  [[nodiscard]] std::filesystem::path SolutionFile() const { return solutionFile_; }

  std::string Solver() const;

 private:
  std::filesystem::path benders_options_file_;
  std::filesystem::path solutionFile_;
  std::string solver_;
};

#endif  // ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
