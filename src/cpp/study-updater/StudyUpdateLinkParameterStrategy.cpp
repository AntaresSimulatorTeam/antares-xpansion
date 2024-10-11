//
// Created by marechaljas on 16/06/22.
//

#include "antares-xpansion/study-updater/StudyUpdateLinkParameterStrategy.h"

#include "antares-xpansion/study-updater/LinkParametersCSVOverwriter.h"
int StudyUpdateLinkParameterStrategy::Update(
    const ActiveLink& link, const std::map<std::string, double>& map) {
  return UpdateLinkDataParameters(link, map);
}

StudyUpdateLinkParameterStrategy::StudyUpdateLinkParameterStrategy(
    const std::filesystem::path& link_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : StudyUpdateStrategy(link_path, logger) {}

std::filesystem::path StudyUpdateLinkParameterStrategy::getLinkdataFilepath(
    ActiveLink const& link_p) const {
  return antares_link_folder_path / link_p.get_linkor() /
         (link_p.get_linkex() + ".txt");
}

int StudyUpdateLinkParameterStrategy::UpdateLinkDataParameters(
    const ActiveLink& link_p,
    const std::map<std::string, double>& investments_p) const {
  auto linkdataFilename_l = getLinkdataFilepath(link_p);
  LinkParametersCSVOverWriter csv_writer(logger_);
  if (!csv_writer.open(linkdataFilename_l)) {
    return 1;
  }

  std::string line_l;
  for (int line_cnt = 0; line_cnt < NUMBER_OF_HOUR_PER_YEAR; ++line_cnt) {
    LinkdataRecord record_l;
    csv_writer.FillRecord(line_l, record_l);
    auto [direct_capacity, indirect_capacity] =
        computeNewCapacities(investments_p, link_p, line_cnt);
    record_l.updateCapacities(direct_capacity, indirect_capacity);
    csv_writer.WriteRow(record_l);
  }

  csv_writer.commit();

  return 0;
}

std::pair<double, double>
StudyUpdateLinkParameterStrategy::computeNewCapacities(
    const std::map<std::string, double>& investments_p,
    const ActiveLink& link_p, int timepoint_p) const {
  double direct_l = link_p.get_already_installed_capacity() *
                    link_p.already_installed_direct_profile(timepoint_p);
  double indirect_l = link_p.get_already_installed_capacity() *
                      link_p.already_installed_direct_profile(timepoint_p);

  const auto& candidates = link_p.getCandidates();
  for (const auto& candidate : candidates) {
    const auto& it_candidate = investments_p.find(candidate.get_name());
    EnsureCandidateInvestmentFound(investments_p, link_p, candidate,
                                   it_candidate);
    double candidate_investment = it_candidate->second;

    direct_l +=
        candidate_investment * candidate.directCapacityFactor(timepoint_p);
    indirect_l +=
        candidate_investment * candidate.indirectCapacityFactor(timepoint_p);
  }
  return std::pair(direct_l, indirect_l);
}
