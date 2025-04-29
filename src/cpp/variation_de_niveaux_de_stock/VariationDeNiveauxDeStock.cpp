
#include "antares-xpansion/variation_de_niveaux_de_stock/VariationDeNiveauxDeStock.h"

#include <algorithm>
#include <execution>
#include <fmt/core.h>
#include <regex>
#include <tbb/global_control.h>
#include <tbb/parallel_for_each.h>
#include <utility>

#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"

/// @brief Constructor
/// @param logger Logger
/// @param writer JsonWriter
/// @param path_to_data Path to the data folder
ValeursUsage::ValeursUsage(Logger logger,
                           std::shared_ptr<Output::JsonWriter> writer,
                           std::filesystem::path path_to_data)
{
    _logger = std::move(logger);
    _writer = std::move(writer);
    xpansionFolderPath = std::move(path_to_data);
}

/// @brief Generates all combinations of constraint values (Cartesian product).
/// @details This function takes a map of constraints, where each constraint is associated
///          with a list of possible values, and produces all possible combinations
///          of those values across the entire set of constraints.
/// @param constraints Map from constraint names to lists of values.
/// @return A vector of maps, where each map is one possible combination of constraint values.
ConstraintCombos ValeursUsage::GenerateConstraintProduct(const ConstraintMap& constraints)
{
    ConstraintCombos areaCombos = {{}};

    for (const auto& [key, values]: constraints)
    {
        ConstraintCombos areaCombo;

        for (const auto& combo: areaCombos)
        {
            for (double val: values)
            {
                std::map<std::string, double> extended = combo;
                extended[key] = val;
                areaCombo.push_back(std::move(extended));
            }
        }

        areaCombos = std::move(areaCombo);
    }

    return areaCombos;
}

/// @brief Generates all combinations of constraint values for all areas in a subproblem.
/// @details This function iterates over each area in the subproblem, generates local combinations
///          of constraint values using GenerateConstraintProduct, and merges them with
///          area-specific names into a complete list of subproblem-level combinations.
/// @param subPbName Name of the subproblem (used to prefix constraint names).
/// @param areas Map from area names to their constraint maps.
/// @return A vector of maps, where each map is a full combination of all constraints
///         (with area-prefixed keys) for the subproblem.
ConstraintCombos ValeursUsage::GenerateSubPbCombos(const std::string& subPbName,
                                                   const AreaConstraintMaps& areas)
{
    ConstraintCombos subPbCombos;
    ConstraintCombos currentCombos = {{}};

    for (const auto& [areaName, constraints]: areas)
    {
        ConstraintCombos newCombos;
        ConstraintCombos localCombos = GenerateConstraintProduct(constraints);

        for (const auto& combo: currentCombos)
        {
            for (const auto& local: localCombos)
            {
                std::map<std::string, double> merged = combo;
                for (const auto& [cst, val]: local)
                {
                    merged[GetConstraintName(subPbName, areaName, cst)] = val;
                }
                newCombos.push_back(merged);
            }
        }

        currentCombos = std::move(newCombos);
    }

    subPbCombos = std::move(currentCombos);
    return subPbCombos;
}

/// @brief Get the path to the subproblem mps file
/// @param subPbName The name of the subproblem
/// @return The path to the subproblem mps file
std::filesystem::path ValeursUsage::GetSubproblemPath(const std::string& subPbName) const
{
    return xpansionFolderPath / "mps" / subPbName;
}

/// @brief Get the name of the constraint in the mps file
/// @param subPbName The name of the subproblem
/// @param area The name of the area
/// @param constraint The name of the constraint
/// @return The name of the constraint in the mps file
std::string ValeursUsage::GetConstraintName(const std::string& subPbName,
                                            const std::string& area,
                                            const std::string& constraint) const
{
    return fmt::format("{}::area<{}>::week<{}>", constraint, area, GetPbInfo(subPbName).week - 1);
}

/// @brief Add a subproblem to the subproblem map and initialize it with the mps file
///        The solver is initialized here
/// @param pbName The name of the subproblem
/// @return The subproblem worker
SubproblemWorkerPtr ValeursUsage::AddSubproblem(const std::string& pbName)
{
    auto subPbWorker = std::make_shared<SubproblemWorker>(GetSubproblemPath(pbName),
                                                          1,
                                                          "XPRESS",
                                                          2,
                                                          solver_log_manager_,
                                                          _logger,
                                                          ProblemsFormat::MPS_FILE);
    return subPbWorker;
}

/// @brief Initialize the subproblems from the mps files in the mps folder
///       and generate the RHS grid values for each subproblem
void ValeursUsage::InitSubProblems()
{
    // Add all subproblems mps files to the subproblem map
    for (const auto& entry: std::filesystem::directory_iterator(xpansionFolderPath / "mps"))
    {
        if (entry.path().extension() == ".mps")
        {
            subPbNames.push_back(entry.path().stem().string() + ".mps");
        }
    }
}

/// @brief Generate the RHS grid values for each subproblem
///        The RHS grid values are generated for each area and each constraint
/// @param subPbName The name of the subproblem
/// @param subPbWorker The subproblem worker
/// @return The RHS grid values for each subproblem
std::map<int, AreaConstraintMaps> ValeursUsage::GenerateRHSGridValues(
  std::string subPbName,
  SubproblemWorkerPtr subPbWorker)
{
    std::map<int, AreaConstraintMaps> gridValues;
    // Compute the grid values using the min and max values of the constraints
    auto generateValues = [&](std::string area,
                              double min,
                              double max,
                              double step,
                              std::string min_cst_name,
                              std::string max_cst_name,
                              double min_efficiency)
    {
        constexpr double epsilon = 0;
        // constexpr double epsilon = 1e-1;

        if (min == 0.0)
        {
            min += epsilon;
        }
        if (max == 1.0)
        {
            max -= epsilon;
        }

        double min_cst = -subPbWorker->get_rhs_value_from_name(
                           GetConstraintName(subPbName, area, min_cst_name))
                         * min_efficiency;
        double max_cst = subPbWorker->get_rhs_value_from_name(
          GetConstraintName(subPbName, area, max_cst_name));

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

        int gridID = std::stoi(tokens[0]);
        std::string pbName = tokens[1];
        std::string areaName = tokens[4];
        std::string cstName = tokens[3];

        double min = std::stod(tokens[5]);
        double max = std::stod(tokens[6]);
        double step = std::stod(tokens[7]);

        std::string minCst = tokens[8];
        std::string maxCst = tokens[9];
        double minEfficiency = std::stod(tokens[10]);

        if (pbName == "all" || pbName == subPbName)
        {
            // Generate values for the subproblem
            gridValues[gridID][areaName][cstName] = generateValues(areaName,
                                                                   min,
                                                                   max,
                                                                   step,
                                                                   minCst,
                                                                   maxCst,
                                                                   minEfficiency);
        }
    }
    return gridValues;
}

/// @brief Get the problem info from the problem name
/// @param pbName The problem name
/// @return The scenario and week of the problem
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

/// @brief Set the constraints RHS values for a given subproblem
/// @param rhsValues The RHS values to set
/// @param subPbWorker The subproblem worker
void ValeursUsage::SetConstraintsRHSValues(const std::map<std::string, double>& rhsValues,
                                           SubproblemWorkerPtr subPbWorker)
{
    for (const auto& [constraintName, value]: rhsValues)
    {
        subPbWorker->fix_rhs_to(constraintName, value);
    }
}

/// @brief Runs the ProcessSubproblem method in parallel for each subproblem
void ValeursUsage::Run()
{
    // sort subPbNames to ensure consistent order
    std::sort(subPbNames.begin(), subPbNames.end());
    ProcessSubproblemsWithPhysicalCores(subPbNames);
}

/// @brief Process a single subproblem
/// @details this function generates all possible combinations of right-hand side (RHS) constraint
///          values using `GenerateSubPbCombos`, applies them to the model via
///          `SetConstraintsRHSValues`, solves the subproblem using `SolveSubproblem`, and stores
///          the resulting cost in the `variationDeNiveauxDeStockData` map indexed by scenario,
///          week, and constraint values.
/// @param subPbName The name of the subproblem
void ValeursUsage::ProcessSubproblem(const std::string& subPbName)
{
    auto subPbWorker = AddSubproblem(subPbName);
    auto currentSubPbAreaConstraints = GenerateRHSGridValues(subPbName, subPbWorker);

    for (const auto& [gridID, subPbAreaConstraints]: currentSubPbAreaConstraints)
    {
        ConstraintCombos subPbCombos = GenerateSubPbCombos(subPbName, subPbAreaConstraints);

        for (const auto& subPbCombo: subPbCombos)
        {
            // Each areaCombo is a std::map<std::string, double> with full variable names
            SetConstraintsRHSValues(subPbCombo, subPbWorker);
            double cost = SolveSubproblem(subPbWorker);

            // No mutex needed if each thread writes to a unique key
            variationDeNiveauxDeStockData[{GetPbInfo(subPbName).scenario,
                                           GetPbInfo(subPbName).week,
                                           subPbCombo}]
              = cost;
        }
    }
    std::cout << "Scenario " << GetPbInfo(subPbName).scenario << " week "
              << GetPbInfo(subPbName).week << std::endl;
}

int get_physical_core_count()
{
    return std::thread::hardware_concurrency() / 2;
}

void ValeursUsage::ProcessSubproblemsWithPhysicalCores(const std::vector<std::string>& subPbNames)
{
    int numPhysicalCores = get_physical_core_count();

    // Limiter TBB au nombre de cœurs physiques
    tbb::global_control limit(tbb::global_control::max_allowed_parallelism, numPhysicalCores);

    // Maintenant utiliser tbb::parallel_for_each
    tbb::parallel_for_each(subPbNames.begin(),
                           subPbNames.end(),
                           [&](const std::string& subPbName) { ProcessSubproblem(subPbName); });
}

/// @brief Solve the subproblem and return the cost
/// @param subPbName The name of the subproblem to solve
/// @return The cost of the subproblem
double ValeursUsage::SolveSubproblem(SubproblemWorkerPtr subPbWorker)
{
    PlainData::SubProblemData subproblem_data;
    Timer subproblem_timer;
    subPbWorker->solve(subproblem_data.lpstatus, ".", "", _writer);
    subPbWorker->get_value(subproblem_data.subproblem_cost);

    subproblem_data.subproblem_timer = subproblem_timer.elapsed();

    return subproblem_data.subproblem_cost;
}

/// @brief Write the output to the json file
void ValeursUsage::WriteOutput()
{
    _writer->write_VariationDeNiveauxDeStock(variationDeNiveauxDeStockData);
    _writer->dump();
}

/// @brief Launch the Stock level variation computation
void ValeursUsage::launch()
{
    std::cout << "Launching Stock level variation" << std::endl;

    InitSubProblems();
    // Time the Run time
    Timer run_timer;
    Run();
    auto run_time = run_timer.elapsed();
    std::cout << "Stock level variation done in " << run_time << " seconds" << std::endl;
    WriteOutput();
}
