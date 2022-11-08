#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>
#include <mutex>

#include "ActiveLinks.h"
#include "Candidate.h"
#include "ProblemGenerationLogger.h"
#include "ProblemModifier.h"
#include "common_lpnamer.h"

const std::string CANDIDATES_INI{"candidates.ini"};
const std::string STRUCTURE_FILE{"structure.txt"};
const std::string MPS_TXT{"mps.txt"};
const std::string STUDY_FILE{"study.antares"};
typedef std::pair<std::string, std::filesystem::path>
    CandidateNameAndMpsFilePath;
typedef unsigned int ColId;
typedef std::map<CandidateNameAndMpsFilePath, ColId> Couplings;

struct ProblemData {
  ProblemData(const std::string& problem_mps, const std::string& variables_txt);
  std::string _problem_mps;
  std::string _variables_txt;
};

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

  void treatloop(const std::filesystem::path& root, Couplings& couplings) const;

 private:
  std::vector<ProblemData> readMPSList(
      const std::filesystem::path& mps_filePath_p) const;

  void treat(const std::filesystem::path& root, ProblemData const&,
             Couplings& couplings) const;

  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  mutable std::mutex coupling_mutex_;
  std::filesystem::path log_file_path_;
};
