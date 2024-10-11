#include "LauncherHelpers.h"

#include <antares-xpansion/helpers/ArchiveReader.h>

#include <filesystem>

#include "Candidate.h"
#include "antares-xpansion/lpnamer/input_reader/CandidatesINIReader.h"
#include "LinkProblemsGenerator.h"
#include "antares-xpansion/lpnamer/input_reader/LinkProfileReader.h"
#include "LogUtils.h"

void treatAdditionalConstraints(
    SolverAbstract::Ptr master_p,
    const AdditionalConstraints &additionalConstraints_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  // add requested binary variables
  addBinaryVariables(master_p, additionalConstraints_p.getVariablesToBinarise(),
                     logger);

  // add the constraints
  for (const auto &[name, additional_constraint] : additionalConstraints_p) {
    addAdditionalConstraint(master_p, additional_constraint, logger);
  }
}

char getConstraintSenseSymbol(
    const AdditionalConstraint &additionalConstraint_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  char rtype;
  if (std::string sign_l = additionalConstraint_p.getSign();
      sign_l == "less_or_equal") {
    rtype = 'L';
  } else if (sign_l == "greater_or_equal") {
    rtype = 'G';
  } else if (sign_l == "equal") {
    rtype = 'E';
  } else {
    (*logger)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION << "FATAL: in addAdditionalConstraint, unknown row type "
        << sign_l << std::endl;
    std::exit(1);
  }
  return rtype;
}

void addAdditionalConstraint(
    SolverAbstract::Ptr master_p,
    const AdditionalConstraint &additionalConstraint_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto newnz = (int)additionalConstraint_p.size();
  int newrows = 1;
  std::vector<char> rtype(newrows);
  std::vector<double> rhs(newrows, additionalConstraint_p.getRHS());
  std::vector<int> mindex(newnz);
  std::vector<double> matval(newnz);
  std::vector<int> matstart(newrows + 1);
  matstart[0] = 0;
  matstart[1] = newnz;

  rtype[0] = getConstraintSenseSymbol(additionalConstraint_p, logger);

  int i = 0;
  for (auto const &[name, coeff] : additionalConstraint_p) {
    int col_index = master_p->get_col_index(name);
    if (col_index == -1) {
      (*logger)(LogUtils::LOGLEVEL::FATAL)
          << LOGLOCATION << "missing variable " << name
          << " used in additional constraint file!\n";
      std::exit(1);
    }
    mindex[i] = col_index;
    matval[i] = coeff;
    i++;
  }

  master_p->add_rows(1, newnz, rtype.data(), rhs.data(), nullptr,
                     matstart.data(), mindex.data(), matval.data());
}

void addBinaryVariables(
    SolverAbstract::Ptr master_p,
    const std::map<std::string, std::string> &variablesToBinarise_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  for (const auto &pairOldNewVarnames : variablesToBinarise_p) {
    int col_index = master_p->get_col_index(pairOldNewVarnames.first);

    if (col_index == -1) {
      (*logger)(LogUtils::LOGLEVEL::FATAL)
          << LOGLOCATION << "missing variable " << pairOldNewVarnames.first
          << " used in additional constraint file!\n";
      std::exit(1);
    }

    master_p->add_cols(
        1, 0, std::vector<double>(1, 0.0).data(), std::vector<int>(2, 0).data(),
        std::vector<int>(0).data(), std::vector<double>(0).data(),
        std::vector<double>(1, 0).data(), std::vector<double>(1, 1e20).data());

    // Changing column type to binary
    master_p->chg_col_type(std::vector<int>(1, master_p->get_ncols() - 1),
                           std::vector<char>(1, 'B'));

    // Changing column name
    master_p->chg_col_name(master_p->get_ncols() - 1,
                           pairOldNewVarnames.second);

    // Add linking constraint
    std::vector<int> matstart(2);
    matstart[0] = 0;
    matstart[1] = 2;

    std::vector<int> matind(2);
    matind[0] = col_index;
    matind[1] = master_p->get_ncols() - 1;

    std::vector<double> matval(2);
    std::vector<double> oldVarUb(1);
    master_p->get_ub(oldVarUb.data(), col_index, col_index);
    matval[0] = 1;
    matval[1] = -oldVarUb[0];

    master_p->add_rows(1, 2, std::vector<char>(1, 'L').data(),
                       std::vector<double>(1, 0.0).data(), nullptr,
                       matstart.data(), matind.data(), matval.data());
    master_p->chg_row_name(
        master_p->get_nrows() - 1,
        "link_" + pairOldNewVarnames.first + "_" + pairOldNewVarnames.second);
  }
}

/**
 * \fn
 * \brief return Active Links Builder
 * \param xpansion_output_dir  path corresponding to the path to the simulation
 * output directory containing the lp directory \return ActiveLinksBuilder
 * object
 */

ActiveLinksBuilder get_link_builders(
    const std::filesystem::path &xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  const auto area_file_name = xpansion_output_dir / "area.txt";

  const auto interco_file_name = xpansion_output_dir / "interco.txt";
  const auto ts_root = xpansion_output_dir / "ts-numbers/ntc";

  CandidatesINIReader candidateReader(interco_file_name, area_file_name,
                                      logger);

  // Get all mandatory path
  auto const xpansion_user_dir =
      xpansion_output_dir / ".." / ".." / "user" / "expansion";
  auto const candidates_file_name = xpansion_user_dir / CANDIDATES_INI;
  auto const capacity_folder = xpansion_user_dir / "capa";

  // Instantiation of candidates
  const auto &candidatesDatas =
      candidateReader.readCandidateData(candidates_file_name);
  const auto &mapLinkProfile = LinkProfileReader(logger).getLinkProfileMap(
      capacity_folder, candidatesDatas);

  return ActiveLinksBuilder(
      candidatesDatas, mapLinkProfile,
      DirectAccessScenarioToChronicleProvider(ts_root, logger), logger);
}