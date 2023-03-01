#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>
#include <mutex>

#include "ActiveLinks.h"
#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "Candidate.h"
#include "FileInBuffer.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationLogger.h"
#include "ProblemModifier.h"
#include "common_lpnamer.h"

const std::string CANDIDATES_INI{"candidates.ini"};
const std::string STRUCTURE_FILE{"structure.txt"};
const std::string STUDY_FILE{"study.antares"};
typedef std::pair<std::string, std::filesystem::path>
    CandidateNameAndMpsFilePath;
typedef unsigned int ColId;
typedef std::map<CandidateNameAndMpsFilePath, ColId> Couplings;

class LinkProblemsGenerator {
 public:
  LinkProblemsGenerator(
      const std::vector<ActiveLink>& links, const std::string& solver_name,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      const std::filesystem::path& log_file_path)
      : _links(links),
        _solver_name(solver_name),
        logger_(logger),
        log_file_path_(log_file_path) {}

  void treatloop(const std::filesystem::path& root,
                 const std::filesystem::path& archivePath,
                 Couplings& couplings);

 private:
  void treat(const std::filesystem::path& root, ProblemData const&,
             Couplings& couplings, ArchiveReader& reader) const;

  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  std::filesystem::path lpDir_ = "";
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  mutable std::mutex coupling_mutex_;
  std::filesystem::path log_file_path_;
};
