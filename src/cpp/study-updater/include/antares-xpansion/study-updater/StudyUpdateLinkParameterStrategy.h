//
// Created by marechaljas on 16/06/22.
//

#pragma once

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/study-updater/StudyUpdateStrategy.h"
class StudyUpdateLinkParameterStrategy : public StudyUpdateStrategy {
 public:
  explicit StudyUpdateLinkParameterStrategy(
      const std::filesystem::path& link_path,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  int Update(const ActiveLink& link,
             const std::map<std::string, double>& map) override;

 private:
  [[nodiscard]] int UpdateLinkDataParameters(
      const ActiveLink& link_p,
      const std::map<std::string, double>& investments_p) const;

 public:
  /*!
   * \brief returns the path to the linkdata file related to a link
   *
   * \param link_p : link for which the datalink file path will be returned
   */
  [[nodiscard]] std::filesystem::path getLinkdataFilepath(
      const ActiveLink& link_p) const;
  /*!
   * \brief computes the new capacities of related to a link
   *
   * \param investment_p : investment to consider for the candidates of the link
   * \param link_p : link for which the capacities will be computed
   * \param timepoint_p : timepoint where the capcities will be computed
   *
   * \return a pair of the computed direct and indirect capacities
   */
  [[nodiscard]] std::pair<double, double> computeNewCapacities(
      const std::map<std::string, double>& investments_p,
      const ActiveLink& link_p, int timepoint_p) const;
};
