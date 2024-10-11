#pragma once

#include <multisolver_interface/SolverAbstract.h>

#include <filesystem>
#include <memory>
#include <mutex>
#include <utility>

#include "antares-xpansion/helpers/ArchiveReader.h"
#include "antares-xpansion/helpers/ArchiveWriter.h"
#include "antares-xpansion/helpers/FileInBuffer.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemVariablesProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemModifier.h"
#include "StringManip.h"
#include "antares-xpansion/lpnamer/input_reader/VariableFileReader.h"

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
      SolverLogManager& solver_log_manager, bool rename_problems)
      : _links(links),
        _solver_name(std::move(solver_name)),
        lpDir_(lpDir),
        logger_(std::move(logger)),
        rename_problems_(rename_problems),
        solver_log_manager_(solver_log_manager) {}

  void treatloop(const std::filesystem::path& root, Couplings& couplings,
                 const std::vector<ProblemData>& mps_list,
                 IProblemWriter* writer);
  void treat(const std::string& problem_name, Couplings& couplings,
             Problem* problem, IProblemVariablesProviderPort* variable_provider,
             IProblemWriter* writer);
  void treat(const std::string& problem_name, Couplings& couplings,
             IProblemProviderPort* problem_provider,
             IProblemVariablesProviderPort* variable_provider,
             IProblemWriter* writer);

 private:
  const std::vector<ActiveLink>& _links;
  std::string _solver_name;
  std::filesystem::path lpDir_ = "";
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
  mutable std::mutex coupling_mutex_;
  bool rename_problems_ = false;
  SolverLogManager& solver_log_manager_;
};
