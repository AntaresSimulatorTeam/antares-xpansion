#include "StudyUpdater.h"

#include "JsonXpansionReader.h"
#include "common_lpnamer.h"

std::filesystem::path StudyUpdater::linksSubfolder_ =
    std::filesystem::path("input") / "links";

StudyUpdater::StudyUpdater(const std::filesystem::path& studyPath_p, const AntaresVersionProvider& antares_version_provider)
    : studyPath_(studyPath_p) {
  linksPath_ = studyPath_ / linksSubfolder_;

  antaresVersion_ = antares_version_provider.getAntaresVersion(studyPath_);
}

int StudyUpdater::getAntaresVersion() const { return antaresVersion_; }

std::filesystem::path StudyUpdater::getLinkdataFilepath(
    ActiveLink const& link_p) const {
  return linksPath_ / link_p.get_linkor() / (link_p.get_linkex() + ".txt");
}

std::pair<double, double> StudyUpdater::computeNewCapacities(
    const std::map<std::string, double>& investments_p,
    const ActiveLink& link_p, int timepoint_p) const {
  double direct_l = link_p.get_already_installed_capacity() *
                    link_p.already_installed_direct_profile(timepoint_p);
  double indirect_l = link_p.get_already_installed_capacity() *
                      link_p.already_installed_direct_profile(timepoint_p);

  const auto& candidates = link_p.getCandidates();
  for (const auto& candidate : candidates) {
    const auto& it_candidate = investments_p.find(candidate.get_name());
    if (it_candidate == investments_p.end()) {
      std::string message = "No investment computed for the candidate " +
                            candidate.get_name() + " on the link " +
                            link_p.get_LinkName();
      throw std::runtime_error(message);
    }
    double candidate_investment = it_candidate->second;

    direct_l += candidate_investment * candidate.directCapacityFactor(timepoint_p);
    indirect_l +=
        candidate_investment * candidate.indirectCapacityFactor(timepoint_p);
  }
  return std::make_pair(direct_l, indirect_l);
}

int StudyUpdater::updateLinkdataFile(
    const ActiveLink& link_p,
    const std::map<std::string, double>& investments_p) const {
  if (antaresVersion_ >= 820) {
    auto link_folder = linksPath_ / link_p.get_linkor() / "capacities";
    auto direct_ntc = link_folder / (link_p.get_linkex() + "_direct.txt");
    auto indirect_ntc = link_folder / (link_p.get_linkex() + "_indirect.txt");
    auto file_direct = std::ofstream(direct_ntc);
    auto file_indirect = std::ofstream(indirect_ntc);
    for (int i = 0; i < NUMBER_OF_HOUR_PER_YEAR; ++i) {
      auto chronicle_capacities_at_time =
          computeNewCapacitiesAllChronicles(investments_p, link_p, i);
      if (!chronicle_capacities_at_time.empty()) {
        for (int j = 0; j < chronicle_capacities_at_time.size() - 1; ++j) {
          auto [direct_capacities_at_time, indirect_capacities_at_time] =
              chronicle_capacities_at_time.at(j);
          file_direct << direct_capacities_at_time << "\t";
          file_indirect << indirect_capacities_at_time << "\t";
        }
        auto [direct_capacities_at_time, indirect_capacities_at_time] =
            *(chronicle_capacities_at_time.end());
        file_direct << direct_capacities_at_time;
        file_indirect << indirect_capacities_at_time;
      }
      file_direct << "\n";
      file_indirect << "\n";
    }
    return 0;
  } else {
    auto linkdataFilename_l = getLinkdataFilepath(link_p);

    std::ifstream inputCsv_l(linkdataFilename_l);
    std::ofstream tempOutCsvFile(linkdataFilename_l.string() + ".tmp");

    if (!tempOutCsvFile.is_open()) {
      std::cout << "ERROR: Error writing " + linkdataFilename_l.string() + "."
                << std::endl;
      return 1;
    }

    bool warned_l = false;
    if (!inputCsv_l.is_open()) {
      std::cout << "WARNING: Missing file to update antares study : "
                << linkdataFilename_l << "."
                << " Unknown valus were populated with 0 in a new created file."
                << std::endl;
      warned_l = true;
    }

    bool isModernVersion_l = (antaresVersion_ >= 700);
    LinkdataRecord record_l(isModernVersion_l);  // csv file fields

    std::string line_l;
    for (int line_cnt = 0; line_cnt < 8760; ++line_cnt) {
      if (std::getline(inputCsv_l, line_l)) {
        record_l.fillFromRow(line_l);
      } else {
        record_l.reset();
        if (!warned_l) {
          std::cout << "WARNING: Missing entries in : " +
                           linkdataFilename_l.string() +
                           ". Missing valus were populated with 0."
                    << std::endl;
          warned_l = true;
        }
      }
      std::pair<double, double> newCapacities_l =
          computeNewCapacities(investments_p, link_p, line_cnt);
      record_l.updateCapacities(newCapacities_l.first, newCapacities_l.second);
      tempOutCsvFile << record_l.to_row("\t") << "\n";
    }

    inputCsv_l.close();
    tempOutCsvFile.close();

    // delete old file and rename the temporarily created file
    std::remove(linkdataFilename_l.string().c_str());
    std::rename((linkdataFilename_l.string() + ".tmp").c_str(),
                linkdataFilename_l.string().c_str());

    return 0;
  }
}

int StudyUpdater::update(std::vector<ActiveLink> const& links_p,
                         std::string const& jsonPath_p) const {
  JsonXpansionReader jsonReader_l;
  jsonReader_l.read(jsonPath_p);

  std::map<std::string, double> solution_l = jsonReader_l.getSolutionPoint();

  return update(links_p, solution_l);
}

int StudyUpdater::update(
    std::vector<ActiveLink> const& links_p,
    const std::map<std::string, double>& investments_p) const {
  int updateFailures_l(0);

  for (const auto& link : links_p) {
    updateFailures_l += updateLinkdataFile(link, investments_p);
  }

  return updateFailures_l;
}

std::vector<std::pair<double,double>> StudyUpdater::computeNewCapacitiesAllChronicles(
    const std::map<std::string, double>& investments_p, const ActiveLink& link_p,
    int timepoint_p) const {
  std::vector<std::pair<double, double>> new_capacities;
  for (int i = 1; i < link_p.number_of_chronicles() + 1; ++i) {
    double direct_l = link_p.get_already_installed_capacity() *
                      link_p.already_installed_direct_profile(i,timepoint_p);
    double indirect_l = link_p.get_already_installed_capacity() *
                        link_p.already_installed_direct_profile(i,timepoint_p);

    const auto& candidates = link_p.getCandidates();
    for (const auto& candidate : candidates) {
      std::cout << "Calculating investment for candidate " << candidate.get_name() << std::endl;
      const auto& it_candidate = investments_p.find(candidate.get_name());
      if (it_candidate == investments_p.end()) {
        std::string message = "No investment computed for the candidate " +
                              candidate.get_name() + " on the link " +
                              link_p.get_LinkName();
        throw std::runtime_error(message);
      }
      double candidate_investment = it_candidate->second;
      std::cout << "investment is " << candidate_investment << std::endl;
      direct_l +=
          candidate_investment * candidate.directCapacityFactor(i,timepoint_p);
      indirect_l +=
          candidate_investment * candidate.indirectCapacityFactor(i,timepoint_p);
    }
    new_capacities.emplace_back(direct_l, indirect_l);
  }
  return new_capacities;
}
