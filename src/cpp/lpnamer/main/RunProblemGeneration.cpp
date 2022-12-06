
#include "RunProblemGeneration.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "Candidate.h"
#include "CandidatesINIReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LinkProfileReader.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "ProblemGenerationExeOptions.h"
#include "solver_utils.h"

void RunProblemGeneration(
    const std::filesystem::path& root, const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    std::filesystem::path archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path) {
  ActiveLinksBuilder linkBuilder = get_link_builders(root, logger);

  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    std::cout << "Invalid formulation argument : argument must be "
                 "\"integer\" or \"relaxed\""
              << std::endl;
    std::exit(1);
  }

  AdditionalConstraints additionalConstraints(logger);
  if (!additionalConstraintFilename_l.empty()) {
    additionalConstraints =
        AdditionalConstraints(additionalConstraintFilename_l, logger);
  }

  Couplings couplings;
  std::string solver_name = "CBC";
  std::vector<ActiveLink> links = linkBuilder.getLinks();
  LinkProblemsGenerator linkProblemsGenerator(links, solver_name, logger,
                                              log_file_path);
  linkProblemsGenerator.treatloop(root, archive_path, couplings);

  MasterGeneration master_generation(root, links, additionalConstraints,
                                     couplings, master_formulation, solver_name,
                                     logger, log_file_path);
}
