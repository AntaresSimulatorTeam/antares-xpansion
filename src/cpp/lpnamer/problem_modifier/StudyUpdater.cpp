#include "StudyUpdater.h"

#include "common_lpnamer.h"
#include "JsonXpansionReader.h"
#include "helpers/Path.h"


std::string StudyUpdater::linksSubfolder_ = static_cast<std::string>( Path("") / "input"  / "links");


StudyUpdater::StudyUpdater(std::string const & studyPath_p):
studyPath_(studyPath_p)
{
    linksPath_ =  studyPath_ + linksSubfolder_;
    readAntaresVersion();
}

int StudyUpdater::getAntaresVersion() const
{
    return antaresVersion_;
}

void StudyUpdater::readAntaresVersion()
{
    std::ifstream studyFile_l (static_cast<std::string> (Path(studyPath_) / STUDY_FILE) );
    std::string line_l;
    const std::string versionProperty_l("version = ");

    while( std::getline(studyFile_l, line_l) )
    {
        size_t pos_l = line_l.find(versionProperty_l);
        if( pos_l != std::string::npos)
        {
            std::stringstream buffer(line_l.substr(pos_l+versionProperty_l.size()));
            buffer >> antaresVersion_;
            return;
        }
    }

    //default
    std::cout << "StudyUpdater::readAntaresVersion : default version 710 returned.\n";
    antaresVersion_ = 710;
}

std::string StudyUpdater::getLinkdataFilepath(ActiveLink const& link_p) const
{
    std::string linkDataFilePath = static_cast<std::string>( Path(linksPath_) / link_p.get_linkor() / (link_p.get_linkex() + ".txt") );
    return linkDataFilePath;
}


std::pair<double, double> StudyUpdater::computeNewCapacities(const std::map<std::string, double>& investments_p, const ActiveLink& link_p, int timepoint_p) const
{
    double direct_l = link_p.get_already_installed_capacity() * link_p.already_installed_direct_profile(timepoint_p);
    double indirect_l = link_p.get_already_installed_capacity() * link_p.already_installed_direct_profile(timepoint_p);

    const auto& candidates = link_p.getCandidates();
    for (const auto& candidate : candidates)
    {
        const auto& it_candidate = investments_p.find(candidate.get_name());
        if (it_candidate == investments_p.end())
        {
            std::string message = "No investment computed for the candidate " + candidate.get_name() + " on the link "+ link_p.get_LinkName();
            throw std::runtime_error(message);
        }
        double candidate_investment = it_candidate->second;

        direct_l    += candidate_investment * candidate.direct_profile(timepoint_p);
        indirect_l  += candidate_investment * candidate.indirect_profile(timepoint_p);
    }
    return std::make_pair(direct_l, indirect_l);
}

int StudyUpdater::updateLinkdataFile(const ActiveLink& link_p, const std::map<std::string, double>& investments_p) const
{
    std::string linkdataFilename_l = getLinkdataFilepath(link_p);

    std::ifstream inputCsv_l(linkdataFilename_l);
    std::ofstream tempOutCsvFile(linkdataFilename_l + ".tmp");

    if (!tempOutCsvFile.is_open())
    {
        std::cout << "ERROR: Error writing " + linkdataFilename_l + "." << std::endl;
        return 1;
    }

    bool warned_l = false;
    if (!inputCsv_l.is_open())
    {
        std::cout << "WARNING: Missing file to update antares study : " << linkdataFilename_l << "."
            << " Unknown valus were populated with 0 in a new created file." << std::endl;
        warned_l = true;
    }

    bool isModernVersion_l = (antaresVersion_ >= 700);
    LinkdataRecord record_l(isModernVersion_l); //csv file fields

    std::string line_l;
    for (int line_cnt = 0; line_cnt < 8760; ++line_cnt)
    {
        if (std::getline(inputCsv_l, line_l))
        {
            record_l.fillFromRow(line_l);
        }
        else
        {
            record_l.reset();
            if (!warned_l)
            {
                std::cout << "WARNING: Missing entries in : " + linkdataFilename_l + ". Missing valus were populated with 0." << std::endl;
                warned_l = true;
            }
        }
        std::pair<double, double> newCapacities_l = computeNewCapacities(investments_p, link_p, line_cnt);
        record_l.updateCapacities(newCapacities_l.first, newCapacities_l.second);
        tempOutCsvFile << record_l.to_row("\t") << "\n";
    }

    inputCsv_l.close();
    tempOutCsvFile.close();

    //delete old file and rename the temporarily created file
    std::remove(linkdataFilename_l.c_str());
    std::rename((linkdataFilename_l + ".tmp").c_str(), linkdataFilename_l.c_str());

    return 0;
}


int StudyUpdater::update(std::vector<ActiveLink> const& links_p, std::string const& jsonPath_p) const
{
    JsonXpansionReader jsonReader_l;
    jsonReader_l.read(jsonPath_p);

    std::map<std::string, double> solution_l = jsonReader_l.getSolutionPoint();

    return update(links_p, solution_l);
}


int StudyUpdater::update(std::vector<ActiveLink> const& links_p, const std::map<std::string, double>& investments_p) const
{
    int updateFailures_l(0);
    
    for (const auto& link : links_p)
    {
        updateFailures_l += updateLinkdataFile(link, investments_p);
    }

    return updateFailures_l;
}