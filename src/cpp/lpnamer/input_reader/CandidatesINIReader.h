//

#ifndef ANTARESXPANSION_CANDIDATESINIREADER_H
#define ANTARESXPANSION_CANDIDATESINIREADER_H

#include <filesystem>
#include <string>
#include <vector>

#include "Candidate.h"
#include "INIReader.h"
#include "IntercoFileReader.h"

class CandidatesINIReader {
 public:
  CandidatesINIReader(const std::filesystem::path& antaresIntercoFile,
                      const std::filesystem::path& areaFile);

  static std::vector<std::string> ReadAreaFile(
      const std::filesystem::path& areaFile);

  std::vector<CandidateData> readCandidateData(
      const std::filesystem::path& candidateFile);

 private:
  bool checkArea(std::string const& areaName_p) const;
  CandidateData readCandidateSection(const std::filesystem::path& candidateFile,
                                     const INIReader& reader,
                                     const std::string& sectionName);

  std::map<std::string, int> _intercoIndexMap;
  std::vector<IntercoFileReader::IntercoFileData> _intercoFileData;
  std::vector<std::string> _areaNames;
};

#endif  // ANTARESXPANSION_CANDIDATESINIREADER_H
