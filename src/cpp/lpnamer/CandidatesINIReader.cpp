//

#include <exception>

#include "helpers/StringUtils.h"

#include "INIReader.h"
#include "CandidatesINIReader.h"


CandidatesINIReader::CandidatesINIReader(const std::string& antaresIntercoFile, const std::string& areaFile) {

    _intercoFileData = ReadAntaresIntercoFile(antaresIntercoFile);
    _areaNames = ReadAreaFile(areaFile);

    for (auto const & intercoFileData : _intercoFileData) {
        //TODO : check if index is available in areaNames
        std::string const & pays_or(_areaNames[intercoFileData.index_pays_origine]);
        std::string const & pays_ex(_areaNames[intercoFileData.index_pays_extremite]);
        std::string linkName = pays_or + " - " + pays_ex;
        _intercoIndexMap[linkName]=intercoFileData.index_interco;
    }

}

std::vector<IntercoFileData> CandidatesINIReader::ReadAntaresIntercoFile(const std::string& antaresIntercoFile) {

    std::vector<IntercoFileData> result;

    std::ifstream interco_filestream(antaresIntercoFile);
    if (!interco_filestream.good()) {
        std::string message = "unable to open " + antaresIntercoFile;
        throw std::runtime_error(message);
    }

    std::string line;
    while (std::getline(interco_filestream, line)) {
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
std::vector<std::string> CandidatesINIReader::ReadAreaFile(const std::string areaFile){

    std::vector<std::string> result;

    std::ifstream area_filestream(areaFile);
    if (!area_filestream.good()) {
        std::string message =  "unable to open " +  areaFile;
        throw std::runtime_error(message);
    }

    std::string line;
    while (std::getline(area_filestream, line)) {
        if (!line.empty() && line.front() != '#') {
            result.push_back(StringUtils::ToLowercase(line));
        }
    }
    return result;
}

std::string getStrVal(const INIReader &reader, const std::string &sectionName, const std::string& key) {
    std::string val = reader.Get(sectionName, key, "NA");
    if ((val != "NA") && (val != "na"))
        return val;
    else
        return "";
}

double getDblVal(const INIReader &reader, const std::string &sectionName, const std::string& key) {

    double d_val(0);

    std::string val = reader.Get(sectionName, key, "NA");
    if (val != "NA") {
        std::stringstream buffer(val);
        buffer >> d_val;;
    }
    return d_val;
}

bool getBoolVal(const INIReader &reader, const std::string &sectionName, const std::string& key) {

    bool result = reader.GetBoolean(sectionName, key, true);
    return result;
}

bool CandidatesINIReader::checkArea(std::string const & areaName_p) const
{
    bool found_l = std::find(_areaNames.cbegin(), _areaNames.cend(), areaName_p) != _areaNames.cend();
    return found_l;
}

std::vector<CandidateData> CandidatesINIReader::readCandidateData(const std::string& candidateFile){
    std::vector<CandidateData> result;

    INIReader reader(candidateFile);
    std::stringstream ss;
    std::set<std::string> sections = reader.Sections();
    for (auto const & sectionName : sections) {

        CandidateData candidateData = readCandidateSection(candidateFile, reader, sectionName);
        result.push_back(candidateData);
    }

    return result;
}

CandidateData CandidatesINIReader::readCandidateSection(const std::string &candidateFile, const INIReader &reader,
                                                     const std::string &sectionName) {
    CandidateData candidateData;
    candidateData.name = StringUtils::ToLowercase(getStrVal(reader,sectionName,"name"));
    candidateData.link_name = StringUtils::ToLowercase(getStrVal(reader, sectionName, "link"));
    size_t i = candidateData.link_name.find(" - ");
    if (i != std::string::npos) {
        candidateData.linkor = candidateData.link_name.substr(0, i);
        candidateData.linkex = candidateData.link_name.substr(i + 3, candidateData.link_name.size());
        if(!checkArea(candidateData.linkor))
        {
            std::string message = "Unrecognized area " + candidateData.linkor + " in section " + sectionName + " in " + candidateFile + ".";
            throw std::runtime_error(message);
        }
        if(!checkArea(candidateData.linkex ))
        {
            std::string message = "Unrecognized area " + candidateData.linkex + " in section " + sectionName + " in " + candidateFile + ".";
            throw std::runtime_error(message);
        }
    }

    //Check if interco is available
    auto it = _intercoIndexMap.find(candidateData.link_name);
    if (it == _intercoIndexMap.end()) {
        std::string message = "cannot link candidate " + candidateData.name + " to interco id";
        throw std::runtime_error(message);
    }
    candidateData.link_id = it->second;
    candidateData.link_profile = getStrVal(reader,sectionName,"link-profile");
    candidateData.installed_link_profile_name = getStrVal(reader, sectionName, "already-installed-link-profile");

    candidateData.annual_cost_per_mw = getDblVal(reader,sectionName,"annual-cost-per-mw");
    candidateData.max_investment = getDblVal(reader,sectionName,"max-investment");
    candidateData.unit_size = getDblVal(reader,sectionName,"unit-size");
    candidateData.max_units = getDblVal(reader,sectionName,"max-units");
    candidateData.already_installed_capacity = getDblVal(reader,sectionName,"already-installed-capacity");
    candidateData.enable = getBoolVal(reader,sectionName, "enable");
    return candidateData;
}
