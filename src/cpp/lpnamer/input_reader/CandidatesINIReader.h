//

#ifndef ANTARESXPANSION_CANDIDATESINIREADER_H
#define ANTARESXPANSION_CANDIDATESINIREADER_H

#include <filesystem>
#include <string>
#include <vector>

#include "Candidate.h"
#include "INIReader.h"
#include "ProblemGenerationLogger.h"

struct IntercoFileData {
  int index_interco;
  int index_pays_origine;
  int index_pays_extremite;
};

class CandidatesINIReader {
 public:
  CandidatesINIReader(
      const std::filesystem::path& antaresIntercoFile,
      const std::filesystem::path& areaFile,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
  CandidatesINIReader(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(std::move(logger)) {}

  std::vector<IntercoFileData> ReadAntaresIntercoFile(
      const std::filesystem::path& antaresIntercoFile);
  std::vector<std::string> ReadAreaFile(const std::filesystem::path& areaFile);

  std::vector<CandidateData> readCandidateData(
      const std::filesystem::path& candidateFile);

 private:
  bool checkArea(std::string const& areaName_p) const;
  CandidateData readCandidateSection(const std::filesystem::path& candidateFile,
                                     const INIReader& reader,
                                     const std::string& sectionName);

  std::map<std::string, int> _intercoIndexMap;
  std::vector<IntercoFileData> _intercoFileData;
  std::vector<std::string> _areaNames;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_CANDIDATESINIREADER_H
