//

#ifndef ANTARESXPANSION_CANDIDATESINIREADER_H
#define ANTARESXPANSION_CANDIDATESINIREADER_H

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "antares-xpansion/lpnamer/model/Candidate.h"
#include "antares-xpansion/lpnamer/input_reader/INIReader.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

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
  explicit CandidatesINIReader(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(logger) {}
  std::vector<IntercoFileData> ReadAntaresIntercoFile(
      const std::filesystem::path& antaresIntercoFile) const;
  std::vector<IntercoFileData> ReadAntaresIntercoFile(
      std::istringstream& antaresIntercoFileInStringStream) const;

  std::vector<IntercoFileData> ReadLineByLineInterco(
      std::istream& stream) const;
  std::vector<CandidateData> readCandidateData(
      const std::filesystem::path& candidateFile) const;

  class InvalidIntercoFile
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };

 private:
  bool checkArea(std::string const& areaName_p) const;
  CandidateData readCandidateSection(const std::filesystem::path& candidateFile,
                                     const INIReader& reader,
                                     const std::string& sectionName,
                                     std::ostringstream& err_msg) const;

  std::map<std::string, int> _intercoIndexMap;
  std::vector<IntercoFileData> _intercoFileData;
  std::vector<std::string> _areaNames;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  void ProcessAreaFile(const std::filesystem::path& areaFile);
};

#endif  // ANTARESXPANSION_CANDIDATESINIREADER_H
