//
// Created by marechaljas on 16/06/22.
//

#pragma once

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "StudyUpdateStrategy.h"
class StudyUpdateLinkCapacitiesStrategy : public StudyUpdateStrategy {
 public:
  explicit StudyUpdateLinkCapacitiesStrategy(
      const std::filesystem::path& link_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  int Update(const ActiveLink& link,
             const std::map<std::string, double>& map) override;

 private:
  int UpdateLinkCapacities(
      const ActiveLink& link_p,
      const std::map<std::string, double>& investments_p) const;
  std::vector<std::pair<double, double>> computeNewCapacitiesAllChronicles(
      const std::map<std::string, double>& investments_p,
      const ActiveLink& link_p, int timepoint_p) const;
};
