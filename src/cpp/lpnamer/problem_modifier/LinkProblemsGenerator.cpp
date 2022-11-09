#include "LinkProblemsGenerator.h"

#include <algorithm>
#include <execution>

#include "IProblemProviderPort.h"
#include "IProblemVariablesProviderPort.h"
#include "IProblemWriter.h"
#include "MPSFileProblemProviderAdapter.h"
#include "ProblemVariablesFileAdapter.h"
#include "ProblemVariablesZipAdapter.h"
#include "ZipProblemProviderAdapter.h"
#include "helpers/StringUtils.h"
#include "solver_utils.h"

ProblemData::ProblemData(const std::string &problem_mps,
                         const std::string &variables_txt)
    : _problem_mps(problem_mps), _variables_txt(variables_txt) {}

std::vector<ProblemData> LinkProblemsGenerator::readMPSList(
    const std::filesystem::path &mps_filePath_p) const {
  std::string line;
  std::vector<ProblemData> result;
  std::ifstream mps_filestream(mps_filePath_p.c_str());
  if (!mps_filestream.good()) {
    (*logger_)(ProblemGenerationLog::LOGLEVEL::FATAL)
        << "unable to open " << mps_filePath_p << std::endl;
    std::exit(1);
  }
  while (std::getline(mps_filestream, line)) {
    std::stringstream buffer(line);
    if (!line.empty() && line.front() != '#') {
      std::string ProblemMps;
      std::string VariablesTxt;
      std::string ConstraintsTxt;

      buffer >> ProblemMps;
      buffer >> VariablesTxt;
      buffer >> ConstraintsTxt;

      ProblemData problemData(ProblemMps, VariablesTxt);

      result.push_back(problemData);
    }
  }

  return result;
}

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
    std::shared_ptr<IProblemWriter> writer,
    std::shared_ptr<IProblemProviderPort> problem_provider,
    std::shared_ptr<IProblemVariablesProviderPort> variable_provider) const {
  std::shared_ptr<Problem> in_prblm =
      problem_provider->provide_problem(_solver_name);

  ProblemVariables problem_variables = variable_provider->Provide();

  solver_rename_vars(in_prblm, problem_variables.variable_names);

  auto problem_modifier = ProblemModifier(logger_);
  in_prblm = problem_modifier.changeProblem(
      in_prblm, _links, problem_variables.ntc_columns,
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
  writer->Write_problem(in_prblm);
}

/**
 * \brief Execute the treat function for each mps elements
 *
 * \param root String corresponding to the path where are located input data
 * \param couplings map of pair of strings associated to an int. Determine the
 * correspondence between optimizer variables and interconnection candidates
 * \return void
 */
void LinkProblemsGenerator::treatloop(const std::filesystem::path &root,
                                      Couplings &couplings,
                                      const std::vector<ProblemData> &mps_list,
                                      std::shared_ptr<IProblemWriter> writer,
                                      std::shared_ptr<ArchiveReader> reader) {
  std::for_each(std::execution::par, mps_list.begin(), mps_list.end(),
                [&](const auto &mps) {
                  auto adapter = std::make_shared<ZipProblemProviderAdapter>(
                      root, mps._problem_mps, reader);
                  auto problem_variables_from_zip_adapter =
                      std::make_shared<ProblemVariablesZipAdapter>(
                          reader, mps, _links, logger_);
                  treat(mps._problem_mps, couplings, writer, adapter,
                        problem_variables_from_zip_adapter);
                });
}

void LinkProblemsGenerator::treatloop_files(
    const std::filesystem::path &root, Couplings &couplings,
    const std::vector<ProblemData> &mps_list,
    std::shared_ptr<IProblemWriter> writer) {
  std::for_each(std::execution::par, mps_list.begin(), mps_list.end(),
                [&](const auto &mps) {
                  auto adapter =
                      std::make_shared<MPSFileProblemProviderAdapter>(
                          root, mps._problem_mps);
                  auto variables_file_adapter =
                      std::make_shared<ProblemVariablesFileAdapter>(
                          mps, _links, logger_, root);
                  treat(mps._problem_mps, couplings, writer, adapter,
                        variables_file_adapter);
                });
}
