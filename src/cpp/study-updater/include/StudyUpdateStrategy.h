//
// Created by marechaljas on 16/06/22.
//

#pragma once

#include "ActiveLinks.h"
#include "LogUtils.h"
#include "ProblemGenerationLogger.h"

class StudyUpdateStrategy {
 public:
  [[nodiscard]] virtual int Update(
      const ActiveLink& link, const std::map<std::string, double>& map) = 0;
  explicit StudyUpdateStrategy(
      const std::filesystem::path& study_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  virtual ~StudyUpdateStrategy() = default;
  class NoInvestmentComputedForTheCandidate
      : public XpansionError<std::runtime_error> {
   public:
    explicit NoInvestmentComputedForTheCandidate(
        const std::string& err_message, const std::string& log_location)
        : XpansionError(err_message, log_location) {}
  };

 protected:
  void EnsureCandidateInvestmentFound(
      const std::map<std::string, double>& investments_p,
      const ActiveLink& link_p, const Candidate& candidate,
      const std::map<std::string, double>::const_iterator& it_candidate) const;

  const std::filesystem::path antares_link_folder_path;

  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};
