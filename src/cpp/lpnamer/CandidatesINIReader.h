//

#ifndef ANTARESXPANSION_CANDIDATESINIREADER_H
#define ANTARESXPANSION_CANDIDATESINIREADER_H

#include <string>
#include <vector>

#include "INIReader.h"
#include "Interco.h"
#include "Candidate.h"

struct IntercoFileData {
    int index_interco;
    int index_pays_origine;
    int index_pays_extremite;
};

class CandidatesINIReader {

public:

    CandidatesINIReader(const std::string& antaresIntercoFile, const std::string& areaFile);

    static std::vector<IntercoFileData> ReadAntaresIntercoFile(const std::string& antaresIntercoFile);
    static std::vector<std::string> ReadAreaFile(const std::string areaFile);

    std::vector<CandidateData> readCandidateData(const std::string& candidateFile);

private:

    bool checkArea(std::string const & areaName_p) const;
    CandidateData  readCandidateSection(const std::string &candidateFile, const INIReader &reader, const std::string &sectionName);

    std::map<std::string,int> _intercoIndexMap;
    std::vector<IntercoFileData> _intercoFileData;
    std::vector<std::string> _areaNames;
};


#endif //ANTARESXPANSION_CANDIDATESINIREADER_H
