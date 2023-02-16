#include "LinkProblemsGenerator.h"

#include <algorithm>
#include <cassert>
#include <execution>

#include "MpsTxtWriter.h"
#include "VariableFileReader.h"
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
void LinkProblemsGenerator::treat(const std::filesystem::path &root,
                                  ProblemData const &problemData,
                                  Couplings &couplings,
                                  ArchiveReader &reader) const {
  // get path of file problem***.mps, variable***.txt and constraints***.txt
  auto const lp_mps_name = lpDir_ / problemData._problem_mps;

  // List of variables
  VariableFileReadNameConfiguration variable_name_config;
  variable_name_config.ntc_variable_name = "ValeurDeNTCOrigineVersExtremite";
  variable_name_config.cost_origin_variable_name =
      "CoutOrigineVersExtremiteDeLInterconnexion";
  variable_name_config.cost_extremite_variable_name =
      "CoutExtremiteVersOrigineDeLInterconnexion";
  auto variableFileContent =
      reader.ExtractFileInStringStream(problemData._variables_txt);
  auto variableReader = VariableFileReader(variableFileContent, _links,
                                           variable_name_config, logger_);

  std::vector<std::string> var_names = variableReader.getVariables();
  std::map<colId, ColumnsToChange> p_ntc_columns =
      variableReader.getNtcVarColumns();
  std::map<colId, ColumnsToChange> p_direct_cost_columns =
      variableReader.getDirectCostVarColumns();
  std::map<colId, ColumnsToChange> p_indirect_cost_columns =
      variableReader.getIndirectCostVarColumns();

  SolverFactory factory;
  auto in_prblm = std::make_shared<Problem>(
      factory.create_solver(_solver_name, log_file_path_));
  reader.ExtractFile(problemData._problem_mps, lpDir_);
  in_prblm->read_prob_mps(lp_mps_name);

  solver_rename_vars(in_prblm, var_names);

  auto problem_modifier = ProblemModifier(logger_);
  in_prblm = problem_modifier.changeProblem(
      std::move(in_prblm), _links, p_ntc_columns, p_direct_cost_columns,
      p_indirect_cost_columns);

  // couplings creation
  for (const ActiveLink &link : _links) {
    for (const Candidate &candidate : link.getCandidates()) {
      if (problem_modifier.has_candidate_col_id(candidate.get_name())) {
        std::lock_guard guard(coupling_mutex_);
        couplings[{candidate.get_name(), lp_mps_name}] =
            problem_modifier.get_candidate_col_id(candidate.get_name());
      }
    }
  }

  in_prblm->write_prob_mps(lp_mps_name);
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
                                  Couplings &couplings, ArchiveReader &reader,
                                  ArchiveWriter &writer) const {
  // get path of file problem***.mps, variable***.txt and constraints***.txt
  auto const lp_mps_name = lpDir_ / problemData._problem_mps;
  treat(root, problemData, couplings, reader);
  writer.AddFileInArchive(lp_mps_name);
  std::filesystem::remove(lp_mps_name);
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
                                      const std::filesystem::path &archivePath,
                                      Couplings &couplings) {
  auto const mps_file_name = root / common_lpnamer::MPS_TXT;

  auto files_mapper = FilesMapper(archivePath);
  auto mpsList = files_mapper.MpsAndVariablesFilesVect();

  lpDir_ = root / "lp";
  auto reader = ArchiveReader(archivePath);
  reader.Open();
  if (zip_mps_) {
    const auto tmpArchiveName = MPS_ZIP_FILE + "-tmp" + ZIP_EXT;
    const auto tmpArchivePath = lpDir_ / tmpArchiveName;
    auto writer = ArchiveWriter(tmpArchivePath);
    writer.Open();
    std::for_each(
        std::execution::par, mpsList.begin(), mpsList.end(),
        [&](const auto &mps) { treat(root, mps, couplings, reader, writer); });
    writer.Close();
    writer.Delete();
    reader.Close();
    reader.Delete();
    std::filesystem::rename(tmpArchivePath, lpDir_ / (MPS_ZIP_FILE + ZIP_EXT));
  } else {
    std::for_each(
        std::execution::par, mpsList.begin(), mpsList.end(),
        [&](const auto &mps) { treat(root, mps, couplings, reader); });
    reader.Close();
    reader.Delete();
  }
}
