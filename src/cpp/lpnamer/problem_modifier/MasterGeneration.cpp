#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"

#include <algorithm>
#include <filesystem>
#include <fmt/format.h>
#include <utility>

#include "antares-xpansion/core/ProblemFormatStream.h"
#include "antares-xpansion/lpnamer/problem_modifier/FileWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterProblemBuilder.h"
#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

MasterGeneration::MasterGeneration(
  std::filesystem::path ouput_path,
  std::string solver_name,
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
  SolverLogManager& solver_log_manager,
  FileWriter& file_writer,
  ProblemsFormat format):
    output_path_(std::move(ouput_path)),
    logger_(std::move(logger)),
    logManager_(solver_log_manager),
    solver_name_(std::move(solver_name)),
    writer_(file_writer),
    format_(format)
{
}

std::vector<Candidate> MasterGeneration::generate(
  const std::vector<ActiveLink>& links,
  const std::string& master_formulation,
  const AdditionalConstraints& additionalConstraints_p) const
{
    auto&& candidates = build_candidates(links);
    write_master_mps(candidates, master_formulation, solver_name_, additionalConstraints_p);
    return candidates;
}

std::vector<Candidate> MasterGeneration::build_candidates(
  const std::vector<ActiveLink>& links) const
{
    std::vector<Candidate> candidates;
    for (const auto& link: links)
    {
        const auto& candidateFromLink = link.getCandidates();
        candidates.insert(candidates.end(), candidateFromLink.begin(), candidateFromLink.end());
    }
    std::sort(candidates.begin(),
              candidates.end(),
              [](const Candidate& cand1, const Candidate& cand2)
              { return cand1.get_name() < cand2.get_name(); });
    return candidates;
}

void MasterGeneration::write_master_mps(const std::vector<Candidate>& candidates,
                                        const std::string& master_formulation,
                                        const std::string& solver_name,
                                        const AdditionalConstraints& additionalConstraints_p) const
{
    auto master_l = MasterProblemBuilder(master_formulation)
                      .build(solver_name, candidates, logManager_);
    treatAdditionalConstraints(master_l, additionalConstraints_p, logger_);
    Problem master_problem(master_l);
    master_problem._name = "master";
    writer_.Write_problem(&master_problem, output_path_ / "lp" / "master");
}
