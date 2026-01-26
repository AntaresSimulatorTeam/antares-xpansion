#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

#include <algorithm>
#include <cmath>

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "include/antares-xpansion/lpnamer/problem_modifier/RenameUtils.h"

namespace AntaresProblemToXpansionProblemTranslator
{
/**
 *
 * @Note: In case of performance issue we can accept non-const lps and work on
 * references to constant and hebdo parts
 */
std::shared_ptr<Problem> translateToXpansionProblem(
  const Antares::Solver::LpsFromAntares& lps,
  unsigned int year,
  unsigned int week,
  const std::string& solver_name,
  const SolverLogManager& solver_log_manager,
  const RenameUtils& renameUtils,
  ProblemGenerationLog::ProblemGenerationLogger* logger)
{
    SolverFactory factory;
    auto problem = std::make_shared<Problem>(
      factory.create_solver(solver_name, solver_log_manager));
    const auto& constant = lps.constantProblemData;
    auto&& hebdo = lps.weeklyProblems.at({year, week});
    problem->_name = hebdo.name;
    problem->mc_year = year;
    problem->week = week;

    std::vector tmp(constant.VariablesCount, 0);

    /** In constant data we have the names of variables and constraints
     * index from hour 0 to hour 167. We need to rename them to
     * correspond to the current week.
     */
    auto renamed_result = renameUtils.rename_week_names(week,
                                                        constant.VariablesMeaning,
                                                        constant.ConstraintsMeaning,
                                                        logger);

    if (!renamed_result)
    {
        if (logger)
        {
            (*logger)(LogUtils::LOGLEVEL::ERR)
              << "Failed to rename week " << week << " in year " << year << std::endl;
        }
        return nullptr;
    }

    const auto& [variables, constraints] = *renamed_result;

    problem->add_cols(constant.VariablesCount,
                      0,
                      hebdo.LinearCost.data(),
                      tmp.data(),
                      {},
                      {},
                      hebdo.Xmin.data(),
                      hebdo.Xmax.data(),
                      variables);

    problem->add_rows(constant.ConstraintesCount,
                      constant.CoeffCount,
                      convertSignToLEG(hebdo.Direction).data(),
                      hebdo.RHS.data(),
                      nullptr,
                      reinterpret_cast<const int*>(constant.Mdeb.data()),
                      reinterpret_cast<const int*>(constant.ColumnIndexes.data()),
                      constant.ConstraintsMatrixCoeff.data(),
                      constraints);
    // On peut ajouter la partie qui renomme les variables ici si on stocke les
    // données du type de variables dans ConstantDataFromAntares, i.e. en
    // définissant une autre implémentation de IProblemVariablesProviderPort
    return problem;
}

std::vector<char> convertSignToLEG(std::span<const char> data)
{
    std::vector<char> LEG_vector;
    // Exclude final '\0' character
    std::ranges::transform(data,
                           std::back_inserter(LEG_vector),
                           [](char c)
                           {
                               if ('=' == c)
                               {
                                   return 'E';
                               }
                               else if ('<' == c)
                               {
                                   return 'L';
                               }
                               else if ('>' == c)
                               {
                                   return 'G';
                               }
                               else
                               {
                                   throw std::runtime_error(LOGLOCATION + "Bad character parsing "
                                                            + c);
                               }
                           });
    return LEG_vector;
}
} // namespace AntaresProblemToXpansionProblemTranslator
