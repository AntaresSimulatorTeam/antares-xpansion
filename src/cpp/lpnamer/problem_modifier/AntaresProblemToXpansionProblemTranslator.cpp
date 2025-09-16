//
// Created by marechaljas on 22/11/22.
//

#include "antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h"

#include <algorithm>
#include <cmath>
#include <regex>

#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

constexpr unsigned int HOURS_IN_A_WEEK = 168;
constexpr unsigned int DAYS_IN_A_WEEK = 7;

static std::string replace_hour_in_name(const std::string& name, int week)
{
    static const std::regex hour_regex(R"(hour<([[:digit:]]+)*>)");
    static const std::regex day_regex(R"(day<([[:digit:]]+)*>)");
    std::smatch match;
    if (std::regex_search(name, match, hour_regex))
    {
        std::string hour_value = match[1]; // La valeur capturée (ici "42")
        return std::regex_replace(
          name,
          hour_regex,
          "hour<" + std::to_string((week - 1) * HOURS_IN_A_WEEK + std::stoi(hour_value)) + ">");
    }
    if (std::regex_search(name, match, day_regex))
    {
        std::string day_value = match[1];
        return std::regex_replace(
          name,
          day_regex,
          "day<" + std::to_string((week - 1) * DAYS_IN_A_WEEK + std::stoi(day_value)) + ">");
    }
    if (std::regex_search(name, match, std::regex(R"(week<([[:digit:]]+)*>)")))
    {
        std::string week_value = match[1];
        return std::regex_replace(name,
                                  std::regex(R"(week<([[:digit:]]+)*>)"),
                                  "week<" + std::to_string(week - 1) + ">");
    }
    throw std::runtime_error(LOGLOCATION + "No [hour|day|week]<...> pattern found in " + name);
}

/**
 * Static variables to store renamed variables and constraints names
 * If we had an object it would be private members
 */
std::unordered_map<int, std::vector<std::string>> variables_names;
std::unordered_map<int, std::vector<std::string>> constraints_names;

void rename_week_names(unsigned int week,
                       const std::vector<std::string>& names,
                       std::unordered_map<int, std::vector<std::string>>& container_names)
{
    /* The numbering is the same for every week N of each year. We only need to compute
     * the renaming once for each week N.
     */
    if (container_names.find(week) == container_names.end())
    {
        std::vector<std::string> renamed_variables;
        renamed_variables.reserve(names.size());
        std::ranges::transform(names,
                               std::back_inserter(renamed_variables),
                               [&week](const auto& n) { return replace_hour_in_name(n, week); });
        container_names.emplace(week, renamed_variables);
    }
}

/**
 *
 * @Note: In case of performance issue we can accept non-const lps and work on
 * references to constant and hebdo parts
 */
std::shared_ptr<Problem> AntaresProblemToXpansionProblemTranslator::translateToXpansionProblem(
  const Antares::Solver::LpsFromAntares& lps,
  unsigned int year,
  unsigned int week,
  const std::string& solver_name,
  SolverLogManager& solver_log_manager)
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
    rename_week_names(week, constant.VariablesMeaning, variables_names);
    rename_week_names(week, constant.ConstraintsMeaning, constraints_names);

    problem->add_cols(constant.VariablesCount,
                      0,
                      hebdo.LinearCost.data(),
                      tmp.data(),
                      {},
                      {},
                      hebdo.Xmin.data(),
                      hebdo.Xmax.data(),
                      variables_names[week]);

    std::span signs(hebdo.Direction.data(), hebdo.Direction.size());
    problem->add_rows(constant.ConstraintesCount,
                      constant.CoeffCount,
                      convertSignToLEG(signs).data(),
                      hebdo.RHS.data(),
                      nullptr,
                      reinterpret_cast<const int*>(constant.Mdeb.data()),
                      reinterpret_cast<const int*>(constant.ColumnIndexes.data()),
                      constant.ConstraintsMatrixCoeff.data(),
                      constraints_names[week]);
    // On peut ajouter la partie qui renomme les variables ici si on stocke les
    // données du type de variables dans ConstantDataFromAntares, i.e. en
    // définissant une autre implémentation de IProblemVariablesProviderPort
    return problem;
}

std::vector<char> AntaresProblemToXpansionProblemTranslator::convertSignToLEG(
  std::span<const char> data)
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
