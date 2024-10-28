//

#include "antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h"

#include <exception>

#include "antares-xpansion/helpers/AreaParser.h"
#include "antares-xpansion/lpnamer/input_reader/INIReader.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"

CandidatesINIReader::CandidatesINIReader(
    const std::filesystem::path &antaresIntercoFile,
    const std::filesystem::path &areaFile,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
    : logger_(logger) {
  _intercoFileData = ReadAntaresIntercoFile(antaresIntercoFile);

  ProcessAreaFile(areaFile);

  for (auto const &intercoFileData : _intercoFileData) {
    // TODO : check if index is available in areaNames
    std::string const &pays_or(_areaNames[intercoFileData.index_pays_origine]);
    std::string const &pays_ex(
        _areaNames[intercoFileData.index_pays_extremite]);
    std::string linkName = pays_or + " - " + pays_ex;
    _intercoIndexMap[linkName] = intercoFileData.index_interco;
  }
}
void CandidatesINIReader::ProcessAreaFile(
    const std::filesystem::path &areaFile) {
  const auto areaFileData = AreaParser::ReadAreaFile(areaFile);
  if (const auto &msg = areaFileData.error_message; !msg.empty()) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << msg;
    throw AreaFileError(
        PrefixMessage(LogUtils::LOGLEVEL::FATAL, logger_->getContext()), msg);
  }
  _areaNames = areaFileData.areas;
}

std::vector<IntercoFileData> CandidatesINIReader::ReadAntaresIntercoFile(
    const std::filesystem::path &antaresIntercoFile) const {
  std::ifstream interco_filestream(antaresIntercoFile);
  if (!interco_filestream.good()) {
    auto loglocation = LOGLOCATION;
    using namespace std::string_literals;
    auto message = "unable to open "s + antaresIntercoFile.string();
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION << message;
    throw InvalidIntercoFile(message, loglocation);
  }

  return ReadLineByLineInterco(interco_filestream);
}

std::vector<IntercoFileData> CandidatesINIReader::ReadAntaresIntercoFile(
    std::istringstream &antaresIntercoFileInStringStream) const {
  return ReadLineByLineInterco(antaresIntercoFileInStringStream);
}

std::vector<IntercoFileData> CandidatesINIReader::ReadLineByLineInterco(
    std::istream &stream) const {
  std::vector<IntercoFileData> result;

  std::string line;
  while (std::getline(stream, line)) {
    std::stringstream buffer(line);
    if (!line.empty() && line.front() != '#') {
      IntercoFileData intercoData;
      buffer >> intercoData.index_interco;
      buffer >> intercoData.index_pays_origine;
      buffer >> intercoData.index_pays_extremite;

      result.push_back(intercoData);
    }
  }
  return result;
}


std::string getStrVal(const INIReader &reader, const std::string &sectionName,
                      const std::string &key) {
  std::string val = reader.Get(sectionName, key, "NA");
  if ((val != "NA") && (val != "na"))
    return val;
  else {
    return "";
  }
}

double getDblVal(const INIReader &reader, const std::string &sectionName,
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

bool getBoolVal(const INIReader &reader, const std::string &sectionName,
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
    const std::filesystem::path &candidateFile) const {
  std::vector<CandidateData> result;

  INIReader reader(candidateFile.string());
  std::ostringstream err_msg;
  const auto starting_pos = err_msg.tellp();
  std::set<std::string> sections = reader.Sections();
  for (auto const &sectionName : sections) {
    CandidateData candidateData =
        readCandidateSection(candidateFile, reader, sectionName, err_msg);
    result.push_back(candidateData);
  }
  err_msg.flush();
  err_msg.seekp(0, std::ios_base::end);
  if (err_msg.tellp() != starting_pos) {
    (*logger_)(LogUtils::LOGLEVEL::FATAL) << err_msg.str();
    std::exit(1);
  }
  return result;
}

CandidateData CandidatesINIReader::readCandidateSection(
    const std::filesystem::path &candidateFile, const INIReader &reader,
    const std::string &sectionName, std::ostringstream &err_msg) const {
  CandidateData candidateData;
  candidateData.name = StringManip::StringUtils::ToLowercase(
      getStrVal(reader, sectionName, "name"));
  candidateData.link_name = StringManip::StringUtils::ToLowercase(
      getStrVal(reader, sectionName, "link"));
  size_t i = candidateData.link_name.find(" - ");
  if (i != std::string::npos) {
    candidateData.linkor = candidateData.link_name.substr(0, i);
    candidateData.linkex =
        candidateData.link_name.substr(i + 3, candidateData.link_name.size());
    if (!checkArea(candidateData.linkor)) {
      err_msg << LOGLOCATION << "Unrecognized area " << candidateData.linkor
              << " in section " << sectionName << " in "
              << candidateFile.string() << "\n";
    }
    if (!checkArea(candidateData.linkex)) {
      err_msg << LOGLOCATION << "Unrecognized area " << candidateData.linkex
              << " in section " << sectionName << " in "
              << candidateFile.string() << "\n";
    }
  }

  // Check if interco is available
  auto it = _intercoIndexMap.find(candidateData.link_name);
  if (it == _intercoIndexMap.end()) {
    err_msg << LOGLOCATION << "cannot link candidate " << candidateData.name
            << " to interco id"
            << "\n";
  }

  if (err_msg.tellp() != 0) {
    return {};
  }
  candidateData.link_id = it->second;
  candidateData.direct_link_profile =
      getStrVal(reader, sectionName, "direct-link-profile");
  candidateData.indirect_link_profile =
      getStrVal(reader, sectionName, "indirect-link-profile");
  candidateData.installed_direct_link_profile_name =
      getStrVal(reader, sectionName, "already-installed-direct-link-profile");
  candidateData.installed_indirect_link_profile_name =
      getStrVal(reader, sectionName, "already-installed-indirect-link-profile");

  candidateData.annual_cost_per_mw =
      getDblVal(reader, sectionName, "annual-cost-per-mw");
  candidateData.max_investment =
      getDblVal(reader, sectionName, "max-investment");
  candidateData.unit_size = getDblVal(reader, sectionName, "unit-size");
  candidateData.max_units = getDblVal(reader, sectionName, "max-units");
  candidateData.already_installed_capacity =
      getDblVal(reader, sectionName, "already-installed-capacity");
  candidateData.enable = getBoolVal(reader, sectionName, "enable");
  return candidateData;
}
