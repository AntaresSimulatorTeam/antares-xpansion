#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>

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
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger)
      : _links(links), _solver_name(solver_name), logger_(std::move(logger)) {}

  void treatloop(const std::filesystem::path& root, Couplings& couplings);

 private:
  std::vector<ProblemData> readMPSList(
      const std::filesystem::path& mps_filePath_p);

  void treat(const std::filesystem::path& root, ProblemData const&,
             Couplings& couplings);

  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};
