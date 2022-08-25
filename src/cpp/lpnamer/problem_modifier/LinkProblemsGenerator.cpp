#include "LinkProblemsGenerator.h"

#include <algorithm>
#include <cassert>
#include <execution>

#include "VariableFileReader.h"
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
    std::cout << "unable to open " << mps_filePath_p << std::endl;
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
void LinkProblemsGenerator::treat(const std::filesystem::path &root,
                                  ProblemData const &problemData,
                                  Couplings &couplings, ArchiveReader &reader) {
  // get path of file problem***.mps, variable***.txt and constraints***.txt
  auto const mps_name = root / problemData._problem_mps;
  auto const var_name = root / problemData._variables_txt;

  // new mps file in the new lp directory
  // std::string const lp_name =
  //     problemData._problem_mps.substr(0, problemData._problem_mps.size() -
  //     4);
  // const auto newMpsFile = (lp_name + ".mps");
  auto const lp_mps_name = lpDir_ / problemData._problem_mps;

  // List of variables
  VariableFileReadNameConfiguration variable_name_config;
  variable_name_config.ntc_variable_name = "ValeurDeNTCOrigineVersExtremite";
  variable_name_config.cost_origin_variable_name =
      "CoutOrigineVersExtremiteDeLInterconnexion";
  variable_name_config.cost_extremite_variable_name =
      "CoutExtremiteVersOrigineDeLInterconnexion";
  auto variableReader = VariableFileReader(
      reader.ExtractFileInStringStream(var_name), _links, variable_name_config);

  std::vector<std::string> var_names = variableReader.getVariables();
  std::map<colId, ColumnsToChange> p_ntc_columns =
      variableReader.getNtcVarColumns();
  std::map<colId, ColumnsToChange> p_direct_cost_columns =
      variableReader.getDirectCostVarColumns();
  std::map<colId, ColumnsToChange> p_indirect_cost_columns =
      variableReader.getIndirectCostVarColumns();

  SolverFactory factory;
  auto in_prblm =
      std::make_shared<Problem>(factory.create_solver(_solver_name));

  reader.ExtractFile(problemData._problem_mps, lpDir_);

  in_prblm->read_prob_mps(lp_mps_name);

  solver_rename_vars(in_prblm, var_names);

  auto problem_modifier = ProblemModifier();
  in_prblm = problem_modifier.changeProblem(
      std::move(in_prblm), _links, p_ntc_columns, p_direct_cost_columns,
      p_indirect_cost_columns);

  // couplings creation
  for (const ActiveLink &link : _links) {
    for (const Candidate &candidate : link.getCandidates()) {
      if (problem_modifier.has_candidate_col_id(candidate.get_name())) {
        couplings[{candidate.get_name(), mps_name}] =
            problem_modifier.get_candidate_col_id(candidate.get_name());
      }
    }
  }

  in_prblm->write_prob_mps(lp_mps_name);
  mpsBufferVector.push_back(FileInBuffer().run(lp_mps_name));
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
                                      Couplings &couplings) {
  auto const mps_file_name = root / MPS_TXT;
  lpDir_ = root / "lp";
  // const auto archivePath = lpDir_ / MPS_ZIP_FILE;
  const auto root_dir_name = root.filename().string();
  std::size_t pos = root_dir_name.find("-Xpansion");
  const auto archivePath =
      root.parent_path() / (root_dir_name.substr(0, pos) + ".zip");
  auto reader = ArchiveReader(archivePath);
  reader.Open();
  auto mpsList = readMPSList(mps_file_name);
  std::for_each(std::execution::seq, mpsList.begin(), mpsList.end(), [&](const auto& mps) {
    treat(root, mps, couplings, reader);
  });
  reader.Close();
  reader.Delete();
  std::filesystem::remove(archivePath);
  auto writer = ArchiveWriter(lpDir_ / "MPS_ZIP_FILE.zip");
  writer.Open();
  writer.AddFilesInArchive(mpsBufferVector);
  writer.Close();
  writer.Delete();
}
