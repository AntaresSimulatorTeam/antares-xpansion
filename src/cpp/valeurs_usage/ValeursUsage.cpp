
#include "antares-xpansion/valeurs_usage/ValeursUsage.h"

#include <fmt/core.h>
#include <regex>
#include <utility>

#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"

ValeursUsage::ValeursUsage(Logger logger,
                           std::shared_ptr<Output::JsonWriter> writer,
                           std::filesystem::path path_to_data)
{
    _logger = std::move(logger);
    _writer = std::move(writer);
    xpansionFolderPath = std::move(path_to_data);
}

// Recursive Cartesian product over constraint sets
void ValeursUsage::GenerateConstraintProduct(
  const ConstraintMap& constraints,
  std::map<std::string, double>& current,
  ConstraintMap::const_iterator it,
  const std::function<void(const std::map<std::string, double>&)>& func)
{
    if (it == constraints.end())
    {
        func(current);
        return;
    }

    const std::string& key = it->first;
    const std::vector<double>& values = it->second;

    for (double val: values)
    {
        current[key] = val;
        GenerateConstraintProduct(constraints, current, std::next(it), func);
    }
}

// Recursive Cartesian product over areas
void ValeursUsage::GenerateAreaProduct(
  const std::string subPbName,
  const AreaConstraintMaps& areas,
  std::map<std::string, double>& current,
  AreaConstraintMaps::const_iterator it,
  const std::function<void(const std::map<std::string, double>&)>& func)
{
    if (it == areas.end())
    {
        func(current);
        return;
    }

    const std::string& areaName = it->first;
    const auto& constraints = it->second;

    std::map<std::string, double> areaCombination;
    GenerateConstraintProduct(
      constraints,
      areaCombination,
      constraints.begin(),
      [&](const std::map<std::string, double>& localCombo)
      {
          // Merge area combination into current with area prefix
          for (const auto& [cst, val]: localCombo)
          {
              current[GetConstraintName(subPbName, areaName, cst)] = val;
          }

          GenerateAreaProduct(subPbName, areas, current, std::next(it), func);

          // Clean up for next iteration
          for (const auto& [cst, _]: localCombo)
          {
              current.erase(GetConstraintName(subPbName, areaName, cst));
          }
      });
}

std::filesystem::path ValeursUsage::GetSubproblemPath(const std::string& subPbName) const
{
    return xpansionFolderPath / "mps" / subPbName;
}

std::string ValeursUsage::GetConstraintName(const std::string& subPbName,
                                            const std::string& area,
                                            const std::string& constraint) const
{
    return fmt::format("{}::area<{}>::week<{}>", constraint, area, GetPbInfo(subPbName).week - 1);
}

void ValeursUsage::AddSubproblem(const std::string& pbName)
{
    subProblems[pbName] = std::make_shared<SubproblemWorker>(GetSubproblemPath(pbName),
                                                             1,
                                                             "XPRESS",
                                                             0,
                                                             solver_log_manager_,
                                                             _logger,
                                                             ProblemsFormat::MPS_FILE);
}

void ValeursUsage::InitSubProblems()
{
    // Add all subproblems mps files to the subproblem map
    for (const auto& entry: std::filesystem::directory_iterator(xpansionFolderPath / "mps"))
    {
        if (entry.path().extension() == ".mps")
        {
            AddSubproblem(entry.path().stem().string());
        }
    }
    GenerateRHSGridValues();
}

void ValeursUsage::GenerateRHSGridValues()
{
    auto generateValues = [&](std::string pbName,
                              std::string area,
                              double min,
                              double max,
                              double step,
                              std::string min_cst_name,
                              std::string max_cst_name,
                              double min_efficiency)
    {
        double min_cst = -subProblems[pbName]->get_rhs_value_from_name(
                           GetConstraintName(pbName, area, min_cst_name))
                         * min_efficiency;
        double max_cst = subProblems[pbName]->get_rhs_value_from_name(
          GetConstraintName(pbName, area, max_cst_name));

        int steps = static_cast<int>((max - min) / step);
        std::vector<double> values;
        values.reserve(steps + 1);

        for (int i = 0; i <= steps; ++i)
        {
            double normalized = min + i * step;
            double value = min_cst + (max_cst - min_cst) * normalized;
            values.push_back(value);
        }

        return values;
    };

    // Read grid.csv file
    std::ifstream file(xpansionFolderPath / "grid.csv");
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;

        std::string tokens[11]; // assuming at least 11 fields
        int i = 0;

        while (std::getline(ss, token, ',') && i < 11)
        {
            tokens[i++] = token;
        }

        std::string pbName = tokens[1];
        std::string areaName = tokens[4];
        std::string cstName = tokens[3];

        double min = std::stod(tokens[5]);
        double max = std::stod(tokens[6]);
        double step = std::stod(tokens[7]);

        std::string minCst = tokens[8];
        std::string maxCst = tokens[9];
        double minEfficiency = std::stod(tokens[10]);

        if (pbName == "all")
        {
            for (const auto& [subPbName, _]: subProblems)
            {
                subPbAreaConstraintsMaps[subPbName][areaName][cstName] = generateValues(
                  subPbName,
                  areaName,
                  min,
                  max,
                  step,
                  minCst,
                  maxCst,
                  minEfficiency);
            }
        }
        else
        {
            subPbAreaConstraintsMaps[pbName][areaName][cstName] = generateValues(pbName,
                                                                                 areaName,
                                                                                 min,
                                                                                 max,
                                                                                 step,
                                                                                 minCst,
                                                                                 maxCst,
                                                                                 minEfficiency);
        }
    }
}

ScenarioAndWeek ValeursUsage::GetPbInfo(const std::string& pbName) const
{
    std::regex re("problem-(\\d+)-(\\d+)--optim-nb-\\d+");
    std::smatch match;
    if (std::regex_search(pbName, match, re))
    {
        return {std::stoi(match[1]), std::stoi(match[2])};
    }
    else
    {
        throw std::runtime_error("Invalid problem name format: " + pbName);
    }
}

void ValeursUsage::SetConstraintsRHSValues(const std::string& pbName,
                                           const std::map<std::string, double>& rhsValues)
{
    for (const auto& [constraintName, value]: rhsValues)
    {
        subProblems[pbName]->fix_rhs_to(constraintName, value);
    }
}

void ValeursUsage::SetConstraintsRHSValuesAndSolvePb()
{
    for (const auto& [subPbName, areasConstraints]: subPbAreaConstraintsMaps)
    {
        std::map<std::string, double> current;
        GenerateAreaProduct(subPbName,
                            areasConstraints,
                            current,
                            areasConstraints.begin(),
                            [&](const std::map<std::string, double>& fullCombination)
                            {
                                SetConstraintsRHSValues(subPbName, fullCombination);
                                std::cout << "Solving subproblem " << subPbName << std::endl;
                                std::cout << "With constraints values: " << std::endl;
                                for (const auto& [constraintName, value]: fullCombination)
                                {
                                    std::cout << constraintName << " = " << value << std::endl;
                                }
                                double cost = SolveSubproblem(subPbName);
                                valeursUsageData[{GetPbInfo(subPbName).scenario,
                                                  GetPbInfo(subPbName).week,
                                                  fullCombination}]
                                  = cost;
                            });
    }
}

double ValeursUsage::SolveSubproblem(const std::string& subPbName)
{
    PlainData::SubProblemData subproblem_data;
    auto worker = subProblems[subPbName];
    Timer subproblem_timer;
    worker->solve(subproblem_data.lpstatus, ".", "", _writer);
    worker->get_value(subproblem_data.subproblem_cost);

    subproblem_data.subproblem_timer = subproblem_timer.elapsed();

    return subproblem_data.subproblem_cost;
}

void ValeursUsage::WriteOutput()
{
    _writer->write_ValeursUsage(valeursUsageData);
    _writer->dump();
}

void ValeursUsage::launch()
{
    std::cout << "Launching valeurs d'usage" << std::endl;

    InitSubProblems();
    SetConstraintsRHSValuesAndSolvePb();
    WriteOutput();
}
