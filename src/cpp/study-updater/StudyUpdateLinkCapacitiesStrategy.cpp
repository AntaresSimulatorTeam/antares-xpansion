//
// Created by marechaljas on 16/06/22.
//

#include "antares-xpansion/study-updater/StudyUpdateLinkCapacitiesStrategy.h"

#include "antares-xpansion/study-updater/LinkCapacitiesCSVWriter.h"

int StudyUpdateLinkCapacitiesStrategy::UpdateLinkCapacities(
    const ActiveLink& link_p,
    const std::map<std::string, double>& investments_p) const {
  LinkCapacitiesCSVWriter capacities_csv_writer(link_p,
                                                antares_link_folder_path);

  for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
    if (auto chronicle_capacities_at_time =
            computeNewCapacitiesAllChronicles(investments_p, link_p, i);
        !chronicle_capacities_at_time.empty()) {
      for (auto [direct_capacities_at_time, indirect_capacities_at_time] :
           chronicle_capacities_at_time) {
        capacities_csv_writer.WriteChronicleTimePoint(
            direct_capacities_at_time, indirect_capacities_at_time);
      }
    }
    capacities_csv_writer.FinishTimePoint();
  }
  return 0;
}
int StudyUpdateLinkCapacitiesStrategy::Update(
    const ActiveLink& link, const std::map<std::string, double>& map) {
  return UpdateLinkCapacities(link, map);
}

std::vector<std::pair<double, double>>
StudyUpdateLinkCapacitiesStrategy::computeNewCapacitiesAllChronicles(
    const std::map<std::string, double>& investments_p,
    const ActiveLink& link_p, int timepoint_p) const {
  std::vector<std::pair<double, double>> new_capacities;
  for (int i = 1; i < link_p.number_of_chronicles() + 1; ++i) {
    double direct_l = link_p.get_already_installed_capacity() *
                      link_p.already_installed_direct_profile(i, timepoint_p);
    double indirect_l = link_p.get_already_installed_capacity() *
                        link_p.already_installed_direct_profile(i, timepoint_p);

    const auto& candidates = link_p.getCandidates();
    for (const auto& candidate : candidates) {
      const auto& it_candidate = investments_p.find(candidate.get_name());
      EnsureCandidateInvestmentFound(investments_p, link_p, candidate,
                                     it_candidate);
      double candidate_investment = it_candidate->second;
      direct_l +=
          candidate_investment * candidate.directCapacityFactor(i, timepoint_p);
      indirect_l += candidate_investment *
                    candidate.indirectCapacityFactor(i, timepoint_p);
    }
    new_capacities.emplace_back(direct_l, indirect_l);
  }
  return new_capacities;
}
StudyUpdateLinkCapacitiesStrategy::StudyUpdateLinkCapacitiesStrategy(
    const std::filesystem::path& link_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : StudyUpdateStrategy(link_path, logger) {}
