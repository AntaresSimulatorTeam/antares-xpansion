#include "StudyUpdater.h"

#include "common_lpnamer.h"
#include "JsonXpansionReader.h"


std::string StudyUpdater::linksSubfolder_ = std::string(PATH_SEPARATOR) + "input" + PATH_SEPARATOR + "links";


StudyUpdater::StudyUpdater(std::string const & studyPath_p):
studyPath_(studyPath_p)
{
    linksPath_ =  studyPath_ + linksSubfolder_;
    readAntaresVersion();
}


StudyUpdater::~StudyUpdater()
{
}

int StudyUpdater::getAntaresVersion() const
{
    return antaresVersion_;
}

void StudyUpdater::readAntaresVersion()
{
    std::ifstream studyFile_l(studyPath_ + PATH_SEPARATOR + STUDY_FILE);
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



std::string StudyUpdater::getLinkdataFilepath(Candidate const & candidate_p) const
{
    std::string result_l = linksPath_ + PATH_SEPARATOR + candidate_p.str("linkor") + PATH_SEPARATOR + candidate_p.str("linkex") + ".txt";
	return result_l;
}


std::pair<double, double> StudyUpdater::computeNewCapacities(double investment_p, Candidate & candidate_p, int timepoint_p) const
{
    //direct capacity
    double direct_l =  candidate_p.already_installed_capacity() * candidate_p.already_installed_profile(timepoint_p, studyPath_, true)
                    + investment_p * candidate_p.profile(timepoint_p, studyPath_, true);

    //indirect capacity
    double indirect_l =  candidate_p.already_installed_capacity() * candidate_p.already_installed_profile(timepoint_p, studyPath_, false)
                    + investment_p * candidate_p.profile(timepoint_p, studyPath_, false);


    return std::make_pair(direct_l, indirect_l);
}


int StudyUpdater::updateLinkdataFile(Candidate candidate_p, double investment_p) const
{
    std::string linkdataFilename_l = getLinkdataFilepath(candidate_p);

    std::ifstream inputCsv_l(linkdataFilename_l);
    std::ofstream tempOutCsvFile(linkdataFilename_l+".tmp");

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

    bool isModernVersion_l = ( antaresVersion_ >= 700 );
    LinkdataRecord record_l(isModernVersion_l); //csv file fields

    std::string line_l;
    for(int line_cnt = 0; line_cnt < 8760; ++line_cnt)
    {
        if( std::getline(inputCsv_l, line_l) )
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
        std::pair<double, double> newCapacities_l = computeNewCapacities(investment_p, candidate_p, line_cnt);
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


int StudyUpdater::update(Candidates const & candidates_p, std::string const & jsonPath_p) const
{
    JsonXpansionReader jsonReader_l;
	jsonReader_l.read(jsonPath_p);

	std::map<std::string, double> solution_l = jsonReader_l.getSolutionPoint();

    return update(candidates_p, solution_l);
}


int StudyUpdater::update(Candidates const & candidates_p, std::map<std::string, double> investments_p) const
{
    int updateFailures_l(0);
	for(auto pairStrCandidate : candidates_p)
	{
		updateFailures_l += updateLinkdataFile(pairStrCandidate.second, investments_p[pairStrCandidate.second.str("name")]);
	}

    return updateFailures_l;
}
