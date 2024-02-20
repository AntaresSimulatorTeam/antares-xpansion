//
// Created by marechaljas on 22/11/22.
//

#include "AntaresProblemToXpansionProblemTranslator.h"

#include <cmath>

#include "LogUtils.h"
#include "multisolver_interface/SolverFactory.h"
#include "solver_utils.h"

std::shared_ptr<Problem>
AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
    const LpsFromAntares& lps, unsigned int year, unsigned int week,
    const std::string& solver_name, SolverLogManager& solver_log_manager) {
  SolverFactory factory;
  auto problem = std::make_shared<Problem>(
      factory.create_solver(solver_name, solver_log_manager));
  const auto& constant = lps._constant;
  const auto& hebdo = lps._hebdo.at({year, week});
  problem->_name = hebdo->name;

  std::vector<int> tmp(constant->NombreDeVariables, 0);
  std::vector<char> coltypes(constant->NombreDeVariables, 'C');

  auto round10 = []<typename T>(T& collection) {
    std::ranges::transform(collection, collection.begin(), [](double v) {
      return round(v * pow(10, 10)) * pow(10, -10);
    });
  };

  round10(hebdo->CoutLineaire);
  round10(hebdo->Xmin);
  round10(hebdo->Xmax);
  round10(hebdo->SecondMembre);
  round10(constant->CoefficientsDeLaMatriceDesContraintes);

  problem->add_cols(constant->NombreDeVariables, 0, hebdo->CoutLineaire.data(),
                    tmp.data(), {}, {}, hebdo->Xmin.data(), hebdo->Xmax.data());

  problem->add_rows(
      constant->NombreDeContraintes, constant->NombreDeCoefficients,
      convertSignToLEG(hebdo->Sens.data()).data(), hebdo->SecondMembre.data(),
      {}, constant->Mdeb.data(), constant->IndicesColonnes.data(),
      constant->CoefficientsDeLaMatriceDesContraintes.data());
  for (int i = 0; i < constant->NombreDeVariables; ++i) {
    problem->chg_col_name(i, hebdo->variables[i]);
  }
  for (int i = 0; i < constant->NombreDeContraintes; ++i) {
    problem->chg_row_name(i, hebdo->constraints[i]);
  }
  auto rows = problem->get_nrows();
  auto cols = problem->get_ncols();
  auto elem = problem->get_nelems();
  // On peut ajouter la partie qui renomme les variables ici si on stocke les
  // données du type de variables dans ConstantDataFromAntares, i.e. en
  // définissant une autre implémentation de IProblemVariablesProviderPort
  return problem;
}
std::vector<char> AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
    char* data) {
  std::vector<char> LEG_vector;
  char c = *data;
  while (c != '\0') {
    if ('=' == c) {
      LEG_vector.push_back('E');
      c = *++data;
      continue;
    } else if ('<' == c) {
      LEG_vector.push_back('L');
      c = *++data;
      continue;
    } else if ('>' == c) {
      LEG_vector.push_back('G');
      c = *++data;
      continue;
    } else {
      throw std::runtime_error(LOGLOCATION + "Bad character parsing " + c);
    }
  }
  return LEG_vector;
}
