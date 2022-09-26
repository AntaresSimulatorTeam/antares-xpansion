#ifndef ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#define ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
#include "ProblemGenerationExeOptions.h"
class FullRunOptionsParser : public ProblemGenerationExeOptions {
 private:
  std::string method_;
  std::filesystem::path benders_options_file_;

 public:
  enum class METHOD { SEQUANTIAL, MPI };

  FullRunOptionsParser();
  virtual ~FullRunOptionsParser() = default;
  METHOD Method() const;
  std::filesystem::path BendersOptionsFile() const {
    return benders_options_file_;
  }
};

#endif  // ANTARES_XPANSION_SRC_CPP_FULL_RUN_FULLRUNOPTIONSPARSER_H
