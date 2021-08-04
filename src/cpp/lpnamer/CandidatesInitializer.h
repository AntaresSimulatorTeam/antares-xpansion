#include "Candidates.h"

#include "Candidates.h"
#include "Candidate.h"
#include "Interconnexion.h"
#include "LinkProfileReader.h"
#include "CandidatesINIReader.h"

class CandidatesInitializer {

public:

    CandidatesInitializer(LinkProfileReader& linkProfileReader, CandidatesINIReader& candidatesIniReader);

    Candidates initializedCandidates(std::string const & candidates_file_name, std::string const & capacity_folder);

    Interconnexions initializedInterconnexions(std::string const& candidates_file_name, std::string const& capacity_folder);

private:

    LinkProfile getOrImportProfile(const std::string &capacitySubfolder, const std::string &profile_name);

    std::map<std::string, LinkProfile> _mapLinkProfile;
    LinkProfileReader& _linkProfileReader;
    CandidatesINIReader&  _candidatesIniReader;
};

