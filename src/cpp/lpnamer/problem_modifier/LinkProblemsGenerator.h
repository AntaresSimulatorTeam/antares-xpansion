#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <utility>

#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "FileInBuffer.h"
#include "IProblemProviderPort.h"
#include "IProblemVariablesProviderPort.h"
#include "IProblemWriter.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationLogger.h"
#include "ProblemModifier.h"
#include "StringManip.h"
#include "VariableFileReader.h"

const std::string CANDIDATES_INI{"candidates.ini"};
const std::string STRUCTURE_FILE{"structure.txt"};
const std::string STUDY_FILE{"study.antares"};
using CandidateNameAndProblemName = std::pair<std::string, std::string>;
using ColId = unsigned int;
using Couplings = std::map<CandidateNameAndProblemName, ColId>;

class LinkProblemsGenerator {
 public:
  LinkProblemsGenerator(
      std::filesystem::path& lpDir, const std::vector<ActiveLink>& links,
      std::string solver_name,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      std::filesystem::path log_file_path,
      bool rename_problems)
      : lpDir_(lpDir),
        _links(links),
        _solver_name(std::move(solver_name)),
        logger_(std::move(logger)),
        log_file_path_(std::move(log_file_path)),
        rename_problems_(rename_problems) {}

  void treatloop(const std::filesystem::path& root, Couplings& couplings,
                 const std::vector<ProblemData>& mps_list,
                 IProblemWriter* writer);
  void treat(const std::string& problem_name, Couplings& couplings,
             Problem* problem, IProblemVariablesProviderPort* variable_provider,
             IProblemWriter* writer) const;
  void treat(const std::string& problem_name, Couplings& couplings,
             IProblemProviderPort* problem_provider,
             IProblemVariablesProviderPort* variable_provider,
             IProblemWriter* writer) const;

 private:
  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  std::filesystem::path lpDir_ = "";
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  mutable std::mutex coupling_mutex_;
  std::filesystem::path log_file_path_;
  bool rename_problems_ = false;
};
