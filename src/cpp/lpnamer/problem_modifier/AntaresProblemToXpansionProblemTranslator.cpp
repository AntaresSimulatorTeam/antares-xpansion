//
// Created by marechaljas on 22/11/22.
//

#include "AntaresProblemToXpansionProblemTranslator.h"

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
  problem->init();
  const auto& constant = lps._constant;
  const auto& hebdo = lps._hebdo.at({year, week});

  std::vector<int> tmp(constant->NombreDeVariables, 0);
  std::vector<char> coltypes(constant->NombreDeVariables, 'C');

  problem->add_cols(constant->NombreDeVariables, 0, hebdo->CoutLineaire.data(),
                    tmp.data(), {}, {}, hebdo->Xmin.data(), hebdo->Xmax.data());
  problem->add_rows(
      constant->NombreDeContraintes, constant->NombreDeCoefficients,
      convertSignToLEG(hebdo->Sens.data()).data(), hebdo->SecondMembre.data(),
      {}, constant->Mdeb.data(), constant->IndicesColonnes.data(),
      constant->CoefficientsDeLaMatriceDesContraintes.data());
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
