#include "LinkProblemsGenerator.h"

#include <algorithm>
#include <execution>

#include "ArchiveProblemWriter.h"
#include "IProblemProviderPort.h"
#include "IProblemWriter.h"
#include "MyAdapter.h"
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
    const std::filesystem::path &root, ProblemData const &problemData,
    Couplings &couplings, ArchiveReader &reader, IProblemWriter *writer,
    IProblemProviderPort *problem_provider) const {
  MyAdapter adapter(root, problemData);
  // get path of file problem***.mps, variable***.txt and constraints***.txt
  auto const mps_name = root / problemData._problem_mps;

  std::istringstream variableFileContent =
      adapter.reader_extract(problemData, reader);

  std::vector<std::string> var_names;
  std::map<colId, ColumnsToChange> p_ntc_columns;
  std::map<colId, ColumnsToChange> p_direct_cost_columns;
  std::map<colId, ColumnsToChange> p_indirect_cost_columns;
  adapter.extract_variables(variableFileContent, var_names, p_ntc_columns,
                            p_direct_cost_columns, p_indirect_cost_columns,
                            _links, logger_);

  adapter.reader_extract_file(problemData, reader, root);

  std::shared_ptr<Problem> in_prblm =
      problem_provider->provide_problem(_solver_name);

  solver_rename_vars(in_prblm, var_names);

  auto problem_modifier = ProblemModifier(logger_);
  in_prblm = problem_modifier.changeProblem(in_prblm, _links, p_ntc_columns,
                                            p_direct_cost_columns,
                                            p_indirect_cost_columns);

  // couplings creation
  for (const ActiveLink &link : _links) {
    for (const Candidate &candidate : link.getCandidates()) {
      if (problem_modifier.has_candidate_col_id(candidate.get_name())) {
        std::lock_guard guard(coupling_mutex_);
        couplings[{candidate.get_name(), mps_name}] =
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
                                      IProblemWriter *writer,
                                      ArchiveReader &reader) {
  std::for_each(std::execution::par, mps_list.begin(), mps_list.end(),
                [&](const auto &mps) {
                  MyAdapter adapter(root, mps);
                  treat(root, mps, couplings, reader, writer, &adapter);
                });
}
