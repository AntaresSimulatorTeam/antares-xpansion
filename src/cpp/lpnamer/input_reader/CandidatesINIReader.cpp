//

#include "CandidatesINIReader.h"

#include <exception>

#include "AreaFileReader.h"
#include "INIReader.h"
#include "IntercoFileReader.h"
#include "helpers/StringUtils.h"

CandidatesINIReader::CandidatesINIReader(
    const std::filesystem::path &antaresIntercoFile,
    const std::filesystem::path &areaFile) {
  _intercoFileData =  IntercoFileReader::ReadAntaresIntercoFile(antaresIntercoFile);
  _areaNames = AreaFileReader::ReadAreaFile(areaFile);

  for (auto const &intercoFileData : _intercoFileData) {
    // TODO : check if index is available in areaNames
    std::string const &pays_or(_areaNames[intercoFileData.index_pays_origine]);
    std::string const &pays_ex(
        _areaNames[intercoFileData.index_pays_extremite]);
    std::string linkName = pays_or + " - " + pays_ex;
    _intercoIndexMap[linkName] = intercoFileData.index_interco;
  }
}

std::string readString(const INIReader &reader, const std::string &sectionName,
                      const std::string &key) {
  std::string val = reader.Get(sectionName, key, "NA");
  if ((val != "NA") && (val != "na"))
    return val;
  else
    return "";
}

double readDouble(const INIReader &reader, const std::string &sectionName,
                 const std::string &key) {
  double d_val(0);

  std::string val = reader.Get(sectionName, key, "NA");
  if (val != "NA") {
    std::stringstream buffer(val);
    buffer >> d_val;
    ;
  }
  return d_val;
}

bool readBool(const INIReader &reader, const std::string &sectionName,
                const std::string &key) {
  bool result = reader.GetBoolean(sectionName, key, true);
  return result;
}

bool CandidatesINIReader::checkArea(std::string const &areaName_p) const {
  bool found_l = std::find(_areaNames.cbegin(), _areaNames.cend(),
                           areaName_p) != _areaNames.cend();
  return found_l;
}

std::vector<CandidateData> CandidatesINIReader::readCandidateData(
    const std::filesystem::path &candidateFile) {
  std::vector<CandidateData> result;

  INIReader reader(candidateFile.string());
  std::stringstream ss;
  std::set<std::string> sections = reader.Sections();
  for (auto const &sectionName : sections) {
    CandidateData candidateData =
        readCandidateSection(candidateFile, reader, sectionName);
    result.push_back(candidateData);
  }

  return result;
}

CandidateData CandidatesINIReader::readCandidateSection(
    const std::filesystem::path &candidateFile, const INIReader &reader,
    const std::string &sectionName) {
  CandidateData candidateData;
  candidateData.name =
      StringUtils::ToLowercase(readString(reader, sectionName, "name"));
  candidateData.link_name =
      StringUtils::ToLowercase(readString(reader, sectionName, "link"));
  size_t i = candidateData.link_name.find(" - ");
  if (i != std::string::npos) {
    candidateData.linkor = candidateData.link_name.substr(0, i);
    candidateData.linkex =
        candidateData.link_name.substr(i + 3, candidateData.link_name.size());
    if (!checkArea(candidateData.linkor)) {
      std::string message = "Unrecognized area " + candidateData.linkor +
                            " in section " + sectionName + " in " +
                            candidateFile.string() + ".";
      throw std::runtime_error(message);
    }
    if (!checkArea(candidateData.linkex)) {
      std::string message = "Unrecognized area " + candidateData.linkex +
                            " in section " + sectionName + " in " +
                            candidateFile.string() + ".";
      throw std::runtime_error(message);
    }
  }

  // Check if interco is available
  auto it = _intercoIndexMap.find(candidateData.link_name);
  if (it == _intercoIndexMap.end()) {
    std::string message =
        "cannot link candidate " + candidateData.name + " to interco id";
    throw std::runtime_error(message);
  }
  candidateData.link_id = it->second;
  candidateData.direct_link_profile =
      readString(reader, sectionName, "direct-link-profile");
  candidateData.indirect_link_profile =
      readString(reader, sectionName, "indirect-link-profile");
  candidateData.installed_direct_link_profile_name =
      readString(reader, sectionName, "already-installed-direct-link-profile");
  candidateData.installed_indirect_link_profile_name = readString(
      reader, sectionName, "already-installed-indirect-link-profile");

  candidateData.annual_cost_per_mw =
      readDouble(reader, sectionName, "annual-cost-per-mw");
  candidateData.max_investment =
      readDouble(reader, sectionName, "max-investment");
  candidateData.unit_size = readDouble(reader, sectionName, "unit-size");
  candidateData.max_units = readDouble(reader, sectionName, "max-units");
  candidateData.already_installed_capacity =
      readDouble(reader, sectionName, "already-installed-capacity");
  candidateData.enable = readBool(reader, sectionName, "enable");
  return candidateData;
}
