//
// Created by marechaljas on 22/11/22.
//

#include "AntaresProblemToXpansionProblemTranslator.h"

#include <algorithm>
#include <cmath>

#include "LogUtils.h"
#include "multisolver_interface/SolverFactory.h"
#include "solver_utils.h"

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

  std::vector<int> tmp(constant.VariablesCount, 0);
  std::vector<char> coltypes(constant.VariablesCount, 'C');

  //roundTo10Digit(constant, hebdo);

  problem->add_cols(constant.VariablesCount, 0, hebdo.LinearCost.data(),
                    tmp.data(), {}, {}, hebdo.Xmin.data(), hebdo.Xmax.data());

  std::span<char> signs(hebdo.Direction.data(), hebdo.Direction.size());
  auto LEG_vector = convertSignToLEG(signs);
  problem->add_rows(
      constant.ConstraintesCount, constant.CoeffCount,
      convertSignToLEG(signs).data(), hebdo.RHS.data(),
      nullptr, reinterpret_cast<const int *>(constant.Mdeb.data()), reinterpret_cast<const int *>(constant.ColumnIndexes.data()),
      constant.ConstraintsMatrixCoeff.data(), {});
  problem->chg_col_names(constant.VariablesCount, hebdo.variables);
  problem->chg_row_names(constant.ConstraintesCount, hebdo.constraints);
//  for (int i = 0; i < constant.VariablesCount; ++i) {
//    problem->chg_col_name(i, hebdo.variables[i]);
//  }
//  for (int i = 0; i < constant.ConstraintesCount; ++i) {
//    problem->chg_row_name(i, hebdo.constraints[i]);
//  }
  auto rows = problem->get_nrows();
  auto cols = problem->get_ncols();
  auto elem = problem->get_nelems();
  // On peut ajouter la partie qui renomme les variables ici si on stocke les
  // données du type de variables dans ConstantDataFromAntares, i.e. en
  // définissant une autre implémentation de IProblemVariablesProviderPort
  return problem;
}
void AntaresProblemToXpansionProblemTranslator::roundTo10Digit(
    Antares::Solver::ConstantDataFromAntares& constant,
    Antares::Solver::WeeklyDataFromAntares& hebdo) {
  auto round10 = [](auto& collection) {
    std::ranges::transform(collection, collection.begin(), [](double v) {
      return round(v * pow(10, 10)) * pow(10, -10);
    });
  };

  round10(hebdo.LinearCost);
  round10(hebdo.Xmin);
  round10(hebdo.Xmax);
  round10(hebdo.RHS);
  round10(constant.ConstraintsMatrixCoeff);
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
