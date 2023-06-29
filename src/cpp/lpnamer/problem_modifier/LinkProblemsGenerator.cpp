#include "LinkProblemsGenerator.h"

#include <algorithm>
#include <execution>
#include <utility>

#include "IProblemProviderPort.h"
#include "IProblemVariablesProviderPort.h"
#include "IProblemWriter.h"
#include "MPSFileProblemProviderAdapter.h"
#include "MpsTxtWriter.h"
#include "ProblemVariablesFileAdapter.h"
#include "ProblemVariablesFromProblemAdapter.h"
#include "ProblemVariablesZipAdapter.h"
#include "VariableFileReader.h"
#include "ZipProblemProviderAdapter.h"
#include "common_lpnamer.h"
#include "helpers/StringUtils.h"
#include "solver_utils.h"

/**
 * \brief That function create new optimization problems with new candidates
 *
 * \param root String corresponding to the path where are located input data
 * \param mps Strings vector with  the list of mps files
 * \param couplings map of pair of strings associated to an int. Determine the
 * correspondence between optimizer variables and interconnection candidates
 * \return void
 */

void LinkProblemsGenerator::treat(
    const std::string &problem_name, Couplings &couplings,
    std::shared_ptr<IProblemProviderPort> problem_provider,
    std::shared_ptr<IProblemVariablesProviderPort> variable_provider,
    std::shared_ptr<IProblemWriter> writer) const {
  std::shared_ptr<Problem> in_prblm =
      problem_provider->provide_problem(_solver_name, log_file_path_);

  treat(problem_name, couplings, in_prblm, std::move(variable_provider),
        writer);
}

void LinkProblemsGenerator::treat(
    const std::string &problem_name, Couplings &couplings,
    std::shared_ptr<Problem> problem,
    std::shared_ptr<IProblemVariablesProviderPort> variable_provider,
    std::shared_ptr<IProblemWriter> writer) const {
  ProblemVariables problem_variables = variable_provider->Provide();

  // solver_rename_vars(problem, problem_variables.variable_names);

  auto problem_modifier = ProblemModifier(logger_);
  auto in_prblm = problem_modifier.changeProblem(
      problem, _links, problem_variables.ntc_columns,
      problem_variables.direct_cost_columns,
      problem_variables.indirect_cost_columns);

  // couplings creation
  for (const ActiveLink &link : _links) {
    for (const Candidate &candidate : link.getCandidates()) {
      if (problem_modifier.has_candidate_col_id(candidate.get_name())) {
        std::lock_guard guard(coupling_mutex_);
        couplings[{candidate.get_name(), problem_name}] =
            problem_modifier.get_candidate_col_id(candidate.get_name());
      }
    }
  }
  auto const lp_mps_name = lpDir_ / in_prblm->_name;
  writer->Write_problem(in_prblm);
}

void LinkProblemsGenerator::treatloop(
    const std::filesystem::path &root, Couplings &couplings,
    const std::vector<ProblemData> &mps_list,
    std::shared_ptr<IProblemWriter> writer,
    bool with_variables_files) {
  std::for_each(
      std::execution::par, mps_list.begin(), mps_list.end(),
      [&](const auto &mps) {
        auto adapter = std::make_shared<MPSFileProblemProviderAdapter>(
            root, mps._problem_mps);
        auto problem = adapter->provide_problem(_solver_name, log_file_path_);
        std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
        if (with_variables_files) {
          variables_provider = std::make_shared<ProblemVariablesFileAdapter>(
              mps, _links, logger_, root);
        } else {
          variables_provider =
              std::make_shared<ProblemVariablesFromProblemAdapter>(
                  problem, _links, logger_);
        }

        treat(mps._problem_mps, couplings, problem, variables_provider, writer);
      });
}
