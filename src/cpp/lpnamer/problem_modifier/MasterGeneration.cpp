#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"

#include <algorithm>
#include <filesystem>
#include <utility>

#include "antares-xpansion/lpnamer/problem_modifier/FileWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterProblemBuilder.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

namespace {
  using ProblemName = std::string;
  using CandidateName = std::string;
  using Structure = std::map<ProblemName, std::map<CandidateName, ColId>>;
}
MasterGeneration::MasterGeneration(
    const std::filesystem::path &rootPath, const std::vector<ActiveLink> &links,
    const AdditionalConstraints &additionalConstraints_p, Couplings &couplings,
    const std::string &master_formulation, const std::string &solver_name,
    std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
    SolverLogManager &solver_log_manager)
    : logger_(std::move(logger))
    , solver_name_(solver_name)
{
  add_candidates(links);
  write_master_mps(rootPath, master_formulation, solver_name,
                   additionalConstraints_p, solver_log_manager);
  write_structure_file(rootPath, couplings);
}

void MasterGeneration::add_candidates(const std::vector<ActiveLink> &links) {
  for (const auto &link : links) {
    const auto &candidateFromLink = link.getCandidates();
    candidates.insert(candidates.end(), candidateFromLink.begin(),
                      candidateFromLink.end());
  }
  std::sort(candidates.begin(), candidates.end(),
            [](const Candidate &cand1, const Candidate &cand2) {
              return cand1.get_name() < cand2.get_name();
            });
}

void MasterGeneration::write_master_mps(
    const std::filesystem::path &rootPath,
    std::string const &master_formulation, std::string const &solver_name,
    const AdditionalConstraints &additionalConstraints_p,
    SolverLogManager &solver_log_manager) const {
  auto master_l = MasterProblemBuilder(master_formulation)
                      .build(solver_name, candidates, solver_log_manager);
  treatAdditionalConstraints(master_l, additionalConstraints_p, logger_);
  Problem master_problem(master_l);
  master_problem._name = "master";
  master_problem.save_prob(rootPath / "lp" / "master");
}

std::filesystem::path FileNameForStructureFile(const std::string& problemName,
                                             std::string solverName) {
  if (problemName == "master") return {"master"};
  return SolverConfig::FileName(problemName, SolverConfig(std::move(solverName)));
}

void MasterGeneration::write_structure_file(
    const std::filesystem::path &rootPath, const Couplings &couplings) const {
  Structure structure;
  for (const auto &[candidate_name_and_mps_filePath, colId] : couplings) {
    structure[candidate_name_and_mps_filePath.second]
          [candidate_name_and_mps_filePath.first] = colId;
  }
  unsigned int i = 0;
  for (auto const &candidate : candidates) {
    structure["master"][candidate.get_name()] = i;
    ++i;
  }

  std::ofstream coupling_file(rootPath / "lp" / STRUCTURE_FILE);
  for (auto const &[mps_file_path, candidates_name_and_colId] : structure) {
    for (auto const &[candidate_name, colId] : candidates_name_and_colId) {
      coupling_file << std::setw(50) <<  FileNameForStructureFile(mps_file_path, solver_name_).string();
      coupling_file << std::setw(50) << candidate_name;
      coupling_file << std::setw(10) << colId;
      coupling_file << std::endl;
    }
  }
  coupling_file.close();
}