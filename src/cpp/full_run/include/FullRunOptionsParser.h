#ifndef ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#include "ProblemGenerationExeOptions.h"
class FullRunOptionsParser : public ProblemGenerationExeOptions {
 private:
  std::string method_;
  std::filesystem::path benders_options_file_;
  std::filesystem::path solutionFile_;

 public:
  enum class METHOD { SEQUANTIAL, MPI };

  FullRunOptionsParser();
  virtual ~FullRunOptionsParser() = default;
  METHOD Method() const;
  std::filesystem::path BendersOptionsFile() const {
    return benders_options_file_;
  }
  std::filesystem::path SolutionFile() const { return solutionFile_; }
};

#endif  // ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
