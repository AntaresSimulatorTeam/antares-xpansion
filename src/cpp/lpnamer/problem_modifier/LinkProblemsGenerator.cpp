#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

#include <algorithm>
#include <execution>
#include <utility>

#include "antares-xpansion/lpnamer/problem_modifier/IProblemProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemVariablesProviderPort.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/MPSFileProblemProviderAdapter.h"
#include "antares-xpansion/lpnamer/input_reader/MpsTxtWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFileAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesZipAdapter.h"
#include "antares-xpansion/lpnamer/input_reader/VariableFileReader.h"
#include "antares-xpansion/lpnamer/problem_modifier/ZipProblemProviderAdapter.h"
#include "antares-xpansion/helpers/solver_utils.h"

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
    IProblemProviderPort *problem_provider,
    IProblemVariablesProviderPort *variable_provider, IProblemWriter *writer) {
  auto in_prblm =
      problem_provider->provide_problem(_solver_name, solver_log_manager_);

  treat(problem_name, couplings, in_prblm.get(), variable_provider, writer);
}

void LinkProblemsGenerator::treat(
    const std::string &problem_name, Couplings &couplings, Problem *problem,
    IProblemVariablesProviderPort *variable_provider, IProblemWriter *writer) {
  ProblemVariables problem_variables = variable_provider->Provide();

  if (rename_problems_) {
    solver_rename_vars(problem, problem_variables.variable_names);
  }
  auto problem_modifier = ProblemModifier(logger_);
  problem_modifier.changeProblem(problem, _links, problem_variables.ntc_columns,
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
  auto const lp_mps_name = lpDir_ / problem_name;
  problem->_name = lp_mps_name.string();
  writer->Write_problem(problem, lp_mps_name);
}

void LinkProblemsGenerator::treatloop(const std::filesystem::path &root,
                                      Couplings &couplings,
                                      const std::vector<ProblemData> &mps_list,
                                      IProblemWriter* writer) {
  std::for_each(
      std::execution::par, mps_list.begin(), mps_list.end(),
      [&](const auto &mps) {
        auto adapter = std::make_unique<MPSFileProblemProviderAdapter>(
            root, mps._problem_mps);
        auto problem =
            adapter->provide_problem(_solver_name, solver_log_manager_);
        std::unique_ptr<IProblemVariablesProviderPort> variables_provider;
        if (rename_problems_) {
          variables_provider = std::make_unique<ProblemVariablesFileAdapter>(
              mps, _links, logger_, root);
        } else {
          variables_provider =
              std::make_unique<ProblemVariablesFromProblemAdapter>(
                  problem, _links, logger_);
        }

        treat(mps._problem_mps, couplings, problem.get(),
              variables_provider.get(), writer);
      });
}
