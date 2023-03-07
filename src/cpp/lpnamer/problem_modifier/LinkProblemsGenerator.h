#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>
#include <mutex>

#include "ArchiveReader.h"
#include "ArchiveWriter.h"
#include "FileInBuffer.h"
#include "IProblemProviderPort.h"
#include "IProblemVariablesProviderPort.h"
#include "IProblemWriter.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationLogger.h"
#include "ProblemModifier.h"
#include "VariableFileReader.h"
#include "common_lpnamer.h"

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
      const std::string& solver_name,
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
      const std::filesystem::path& log_file_path)
      : lpDir_(lpDir),
        _links(links),
        _solver_name(solver_name),
        logger_(logger),
        log_file_path_(log_file_path) {}

  void treatloop(const std::filesystem::path& root, Couplings& couplings,
                 const std::vector<ProblemData>& mps_list,
                 std::shared_ptr<IProblemWriter> writer);
  void treat(const std::string& problem_name, Couplings& couplings,
             std::shared_ptr<Problem> problem,
             std::shared_ptr<IProblemVariablesProviderPort> variable_provider,
             std::shared_ptr<IProblemWriter> writer) const;
  void treat(const std::string& problem_name, Couplings& couplings,
             std::shared_ptr<IProblemProviderPort> problem_provider,
             std::shared_ptr<IProblemVariablesProviderPort> variable_provider,
             std::shared_ptr<IProblemWriter> writer) const;

 private:
  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  std::filesystem::path lpDir_ = "";
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  mutable std::mutex coupling_mutex_;
  std::filesystem::path log_file_path_;

 public:
};
