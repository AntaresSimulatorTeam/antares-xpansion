//
// Created by marechaljas on 22/11/22.
//

#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

#include <algorithm>
#include <cmath>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"

/**
 *
 * @Note: In case of performance issue we can accept non-const lps and work on
 * references to constant and hebdo parts
 */
std::shared_ptr<Problem>
AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
    const Antares::Solver::LpsFromAntares& lps, unsigned int year, unsigned int week,
    const std::string& solver_name, SolverLogManager& solver_log_manager) {
  SolverFactory factory;
  auto problem = std::make_shared<Problem>(
      factory.create_solver(solver_name, solver_log_manager));
  auto constant = lps.constantProblemData;
  auto hebdo = lps.weeklyProblems.at({year, week});
  problem->_name = hebdo.name;
  problem->mc_year = year;
  problem->week = week;

  std::vector<int> tmp(constant.VariablesCount, 0);
  std::vector<char> coltypes(constant.VariablesCount, 'C');

  problem->add_cols(constant.VariablesCount, 0, hebdo.LinearCost.data(),
                    tmp.data(), {}, {}, hebdo.Xmin.data(), hebdo.Xmax.data());

  std::span<char> signs(hebdo.Direction.data(), hebdo.Direction.size());
  auto LEG_vector = convertSignToLEG(signs);
  problem->add_rows(
      constant.ConstraintesCount, constant.CoeffCount,
      convertSignToLEG(signs).data(), hebdo.RHS.data(),
      nullptr, reinterpret_cast<const int *>(constant.Mdeb.data()), reinterpret_cast<const int *>(constant.ColumnIndexes.data()),
      constant.ConstraintsMatrixCoeff.data(), {});
  for (int i = 0; i < constant.VariablesCount; ++i) {
    problem->chg_col_name(i, hebdo.variables[i]);
  }
  for (int i = 0; i < constant.ConstraintesCount; ++i) {
    problem->chg_row_name(i, hebdo.constraints[i]);
  }
  // On peut ajouter la partie qui renomme les variables ici si on stocke les
  // données du type de variables dans ConstantDataFromAntares, i.e. en
  // définissant une autre implémentation de IProblemVariablesProviderPort
  return problem;
}

std::vector<char> AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
    std::span<char> data) {
  std::vector<char> LEG_vector;
  //Exclude final '\0' character
  std::ranges::transform(data, std::back_inserter(LEG_vector), [](char c) {
    if ('=' == c) {
      return 'E';
    } else if ('<' == c) {
      return 'L';
    } else if ('>' == c) {
      return 'G';
    } else {
      throw std::runtime_error(LOGLOCATION + "Bad character parsing " + c);
    }
  });
  return LEG_vector;
}
