
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"

#include <algorithm>
#include <execution>
#include <fmt/core.h>
#include <functional>
#include <ranges>
#include <regex>
#include <tbb/global_control.h>
#include <tbb/parallel_for_each.h>
#include <utility>

#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/helpers/Timer.h"

std::atomic<int> totalSimplexIter = 0;     ///< Total number of simplex iterations
std::atomic<double> totalSubPbTimer = 0;   ///< Total time spent solving subproblems
std::atomic<double> totalPbModifTimer = 0; ///< Total time spent modifying subproblems

/// @brief Constructor
/// @param logger Logger
/// @param writer JsonWriter
/// @param path_to_mps Path to the data folder
/// @param gridDefinition GridCollection containing the grids to evaluate
/// @param reservoirManagement Reservoir management
/// @param data_format Format of the data (MPS or LP)
/// @param nbThreads Number of threads to use
GridEvaluator::GridEvaluator(Logger logger,
                             std::shared_ptr<Output::JsonWriter> writer,
                             std::filesystem::path path_to_mps,
                             GridDefinition& gridDefinition,
                             const ReservoirManagement& reservoirManagement,
                             ProblemsFormat data_format,
                             int nbThreads = 1):
    gridDefinition(gridDefinition),
    reservoirManagement(reservoirManagement),
    problemsFormat(data_format),
    nbThreads(nbThreads)
{
    this->logger = std::move(logger);
    this->writer = std::move(writer);
    this->mpsPath = std::move(path_to_mps);
}

/// @brief Generates all combinations of constraint values (Cartesian product).
/// @details This function takes a map of constraints, where each constraint is associated
///          with a list of possible values, and produces all possible combinations
///          of those values across the entire set of constraints.
/// @param constraints Map from constraint names to lists of values.
/// @return A vector of maps, where each map is one possible combination of constraint values.
ConstraintCombos GridEvaluator::GenerateConstraintProduct(const ConstraintMap& constraints)
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
ConstraintCombos GridEvaluator::GenerateSubPbCombos(const std::string& subPbName,
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

/// @brief Flatten a multi-dimensional index to a 1D index
/// @param coord The multi-dimensional index
/// @param dims The dimensions of the array
/// @return The 1D index
int flattenIndex(const std::vector<int>& coord, const std::vector<int>& dims)
{
    int index = 0;
    int stride = 1;
    for (int i = dims.size() - 1; i >= 0; --i)
    {
        index += coord[i] * stride;
        stride *= dims[i];
    }
    return index;
}

/// @brief Generate ND zigzag order based on parity of upper dimensions
/// @param dims The dimensions of the array
/// @return The zigzag order
std::vector<size_t> generateZigzagOrder(const std::vector<int>& dims)
{
    const int ndims = dims.size();
    std::vector<int> current(ndims, 0);
    std::vector<size_t> linearOrder;

    std::function<void(int)> recurse = [&](int level)
    {
        if (level == ndims)
        {
            linearOrder.push_back(flattenIndex(current, dims));
            return;
        }

        bool reverse = false;
        for (int i = 0; i < level; ++i)
        {
            if (current[i] % 2 == 1)
            {
                reverse = !reverse;
            }
        }

        if (!reverse)
        {
            for (int i = 0; i < dims[level]; ++i)
            {
                current[level] = i;
                recurse(level + 1);
            }
        }
        else
        {
            for (int i = dims[level] - 1; i >= 0; --i)
            {
                current[level] = i;
                recurse(level + 1);
            }
        }
    };

    recurse(0);
    return linearOrder;
}

/// @brief Reorder the vector of Points according to the ND zigzag pattern
/// @param dims The dimensions of the array
/// @param points The vector of Points to reorder
/// @return The reordered vector of Points
std::vector<Point> reorderZigzagND(const std::vector<int>& dims, const std::vector<Point>& points)
{
    std::vector<size_t> order = generateZigzagOrder(dims);
    std::vector<Point> reordered;
    reordered.reserve(order.size());

    for (size_t idx: order)
    {
        reordered.push_back(points[idx]);
    }

    return reordered;
}

/// @brief Get the path to the subproblem mps file
/// @param subPbName The name of the subproblem
/// @return The path to the subproblem mps file
std::filesystem::path GridEvaluator::GetSubproblemPath(const std::string& subPbName) const
{
    return mpsPath / subPbName;
}

/// @brief Get the name of the constraint in the mps file
/// @param subPbName The name of the subproblem
/// @param area The name of the area
/// @param constraint The name of the constraint
/// @return The name of the constraint in the mps file
std::string GridEvaluator::GetConstraintName(const std::string& subPbName,
                                             const std::string& area,
                                             const std::string& constraint) const
{
    return fmt::format("{}::area<{}>::week<{}>", constraint, area, GetPbInfo(subPbName).week - 1);
}

/// @brief Add a subproblem to the subproblem map and initialize it with the mps file
///        The solver is initialized here
/// @param pbName The name of the subproblem
/// @return The subproblem worker
SubproblemWorkerPtr GridEvaluator::AddSubproblem(const std::string& pbName)
{
    auto subPbWorker = std::make_shared<SubproblemWorker>(GetSubproblemPath(pbName),
                                                          1,
                                                          "XPRESS",
                                                          2,
                                                          solver_log_manager,
                                                          logger,
                                                          problemsFormat);
    return subPbWorker;
}

/// @brief Get the list of subproblems names to be used in the gridDefinition
std::vector<std::string> GridEvaluator::InitSubProblems(const GridDefinition& gridDefinition)
{
    // Add all subproblems mps files to the subproblem map if they are used in the grid.csv file
    std::string extension = problemsFormat == ProblemsFormat::MPS_FILE ? ".mps" : ".svf";
    std::vector<std::string> subPbNames;
    for (const auto& entry: std::filesystem::directory_iterator(mpsPath))
    {
        if (entry.path().extension() == extension
            && gridDefinition.isSubproblemUsed(entry.path().stem().string()))
        {
            std::string subPbName = entry.path().stem().string();
            subPbNames.push_back(subPbName);
            nbScenarios = std::max(nbScenarios, GetPbInfo(subPbName).scenario);
        }
    }
    return subPbNames;
}

/// @brief Generate the RHS grid values for each subproblem
///        The RHS grid values are generated for each area and each constraint
/// @param subPbName The name of the subproblem
/// @param gridDefinition The grid definition used to generate the RHS grid values
/// @param subPbWorker The subproblem worker
/// @return The RHS grid values for each subproblem
AreaConstraintMaps GridEvaluator::GenerateRHSGridValues(std::string subPbName,
                                                        GridDefinition& gridDefinition,
                                                        SubproblemWorkerPtr subPbWorker)
{
    AreaConstraintMaps gridValues;
    // Compute the grid values using the min and max values of the constraints
    auto computeValues = [&](GridElement& gridElement)
    {
        constexpr double epsilon = 0;
        // constexpr double epsilon = 1e-2;

        if (gridElement.min == 0.0)
        {
            gridElement.min += epsilon;
        }
        if (gridElement.max == 1.0)
        {
            gridElement.max -= epsilon;
        }

        double min_cst = -subPbWorker->get_rhs_value_from_name(
                           GetConstraintName(subPbName, gridElement.area, gridElement.min_cst))
                         * gridElement.min_efficiency;
        double max_cst = subPbWorker->get_rhs_value_from_name(
          GetConstraintName(subPbName, gridElement.area, gridElement.max_cst));

        int steps = static_cast<int>((gridElement.max - gridElement.min) / gridElement.step);

        for (int i = 0; i <= steps; ++i)
        {
            double normalized = gridElement.min + i * gridElement.step;
            double value = min_cst + (max_cst - min_cst) * normalized;
            gridElement.values[GetPbInfo(subPbName)].insert(value);
        }

        return gridElement.values[GetPbInfo(subPbName)];
    };

    for (auto& gridElement: gridDefinition.gridElements)
    {
        if (gridElement.problemName == subPbName || gridElement.problemName == "all")
        {
            gridValues[gridElement.area][gridElement.name] = computeValues(gridElement);
        }
    }
    return gridValues;
}

/// @brief Get the problem info from the problem name
/// @param pbName The problem name
/// @return The scenario and week of the problem
ScenarioAndWeek GridEvaluator::GetPbInfo(const std::string& pbName) const
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
void GridEvaluator::SetConstraintsRHSValues(const std::map<std::string, double>& rhsValues,
                                            SubproblemWorkerPtr subPbWorker)
{
    for (const auto& [constraintName, value]: rhsValues)
    {
        subPbWorker->fix_rhs_to(constraintName, value);
    }
}

/// @brief Runs the ProcessSubproblem method in parallel for each subproblem
/// @param subPbNames The names of the subproblems to process
/// @param gridDefinition The grid definition to use to generate the grid afterwards
void GridEvaluator::Run(const std::vector<std::string>& subPbNames, GridDefinition& gridDefinition)
{
    ProcessGridParallel(subPbNames, gridDefinition, nbThreads);
}

/// @brief Process a single subproblem
/// @details this function generates all possible combinations of right-hand side (RHS)
/// constraint
///          values using `GenerateSubPbCombos`, applies them to the model via
///          `SetConstraintsRHSValues`, solves the subproblem using `SolveSubproblem`, and
///          stores the resulting cost in the `variationDeNiveauxDeStockData` map indexed by
///          scenario, week, and constraint values.
/// @param subPbName The name of the subproblem
/// @param gridDefinition The grid definition to use to generate the grid
void GridEvaluator::ProcessSubproblem(const std::string& subPbName, GridDefinition& gridDefinition)
{
    // Print loading time
    auto start = std::chrono::high_resolution_clock::now();
    auto subPbWorker = AddSubproblem(subPbName);
    std::cout << "Loading subproblem : " << subPbName << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Loading time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms"
              << std::endl;
    auto subPbAreaConstraints = GenerateRHSGridValues(subPbName, gridDefinition, subPbWorker);

    std::cout << "Processing subproblem : scenario " << GetPbInfo(subPbName).scenario << " week "
              << GetPbInfo(subPbName).week << std::endl;

    std::vector<int> dims;

    for (const auto& [_, constraints]: subPbAreaConstraints)
    {
        std::transform(constraints.begin(),
                       constraints.end(),
                       std::back_inserter(dims),
                       [](const auto& constraints) { return constraints.second.size(); });
    }

    ConstraintCombos subPbCombos = GenerateSubPbCombos(subPbName, subPbAreaConstraints);
    subPbCombos = reorderZigzagND(dims, subPbCombos);

    int size = subPbCombos.size();
    int i = 0;
    for (const auto& subPbCombo: subPbCombos)
    {
        i++;
        std::cout << "Processing gridPoint " << i << "/" << size << std::endl;
        // Each areaCombo is a std::map<std::string, double> with full variable names
        Timer timer;
        SetConstraintsRHSValues(subPbCombo, subPbWorker);
        totalPbModifTimer += timer.elapsed();
        for (const auto& [constraintName, value]: subPbCombo)
        {
            std::cout << constraintName << " " << value << std::endl;
        }
        double cost = SolveSubproblem(subPbWorker);

        variationDeNiveauxDeStockData
          .insert({subPbCombo, GetPbInfo(subPbName).scenario, GetPbInfo(subPbName).week}, cost);
        std::cout << "Cost: " << cost << std::endl;
    }
}

/// @brief Process the subproblems in parallel using TBB over nbThreads
/// @param subPbNames The list of subproblems names to process
/// @param gridDefinition The grid definition to use
/// @param nbThreads The number of threads to use
void GridEvaluator::ProcessGridParallel(const std::vector<std::string>& subPbNames,
                                        GridDefinition& gridDefinition,
                                        int nbThreads)
{
    // Limiter TBB au nombre de cœurs physiques
    tbb::global_control limit(tbb::global_control::max_allowed_parallelism, nbThreads);

    tbb::parallel_for_each(subPbNames.begin(),
                           subPbNames.end(),
                           [&gridDefinition, this](const std::string& subPbName)
                           { ProcessSubproblem(subPbName, gridDefinition); });
}

/// @brief Solve the subproblem and return the cost
/// @param subPbWorker The subproblem worker
/// @return The cost of the subproblem
double GridEvaluator::SolveSubproblem(SubproblemWorkerPtr subPbWorker)
{
    PlainData::SubProblemData subproblem_data;
    Timer subproblem_timer;
    subPbWorker->solve(subproblem_data.lpstatus, ".", "", writer);
    subPbWorker->get_value(subproblem_data.subproblem_cost);
    subproblem_data.subproblem_timer = subproblem_timer.elapsed();

    int nbSimplexIter;
    subPbWorker->get_splex_num_of_ite_last(nbSimplexIter);
    std::cout << "nb simplex : " << nbSimplexIter << " / in " << subproblem_data.subproblem_timer
              << " seconds" << std::endl;
    totalSimplexIter += nbSimplexIter;
    totalSubPbTimer += subproblem_data.subproblem_timer;

    return subproblem_data.subproblem_cost;
}

/// @brief Launch the Stock level variation computation
/// @return The stock level variation results
std::map<Output::PointWeekScenarioKey, double> GridEvaluator::ComputeRewards()
{
    std::cout << "Launching Stock level variation" << std::endl;

    // Time the Run time
    Timer run_timer;

    auto subPbNames = InitSubProblems(gridDefinition);
    Run(subPbNames, gridDefinition);

    auto run_time = run_timer.elapsed();
    std::cout << "Stock level variation done in " << run_time << " seconds and " << totalSimplexIter
              << " simplex iterations" << std::endl;
    std::cout << "Time solving subproblems (accumulated by each thread) : " << totalSubPbTimer
              << " seconds" << std::endl;

    return variationDeNiveauxDeStockData.get();
}

/// @brief Get the index of the value in the set
/// @param X the set
/// @param x_query the value to look for
/// @return the index of the value found in the set
size_t rank_in_set(const std::set<double>& X, double x_query)
{
    auto it = X.find(x_query);
    if (it == X.end())
    {
        throw std::invalid_argument("x_query not found in X");
    };
    return std::distance(X.begin(), it);
}

template<typename XContainer, typename YContainer>
double interpolate(const XContainer& X, const YContainer& Y, double x_query)
{
    if (X.size() != Y.size() || X.empty())
    {
        throw std::invalid_argument("X and Y must have the same size and not be empty");
    }

    auto x_begin = X.begin();
    auto x_end = X.end();

    // Clamp below range
    if (x_query <= *x_begin)
    {
        return *Y.begin();
    }

    // Clamp above range
    auto last = std::prev(x_end);
    if (x_query >= *last)
    {
        return *std::prev(Y.end());
    }

    // Find first element >= x_query
    auto it_upper = std::lower_bound(x_begin, x_end, x_query);

    if (it_upper != x_end && *it_upper == x_query)
    {
        // Exact match
        auto y_it = Y.begin();
        std::advance(y_it, std::distance(x_begin, it_upper));
        return *y_it;
    }

    // Otherwise interpolate between previous and it_upper
    auto it_lower = std::prev(it_upper);

    double x0 = *it_lower;
    double x1 = *it_upper;

    auto y_it_lower = Y.begin();
    std::advance(y_it_lower, std::distance(x_begin, it_lower));

    auto y_it_upper = Y.begin();
    std::advance(y_it_upper, std::distance(x_begin, it_upper));

    double y0 = *y_it_lower;
    double y1 = *y_it_upper;

    return y0 + (y1 - y0) * (x_query - x0) / (x1 - x0);
}

auto linspace(double start, double end, int num)
{
    if (num <= 0)
    {
        return std::vector<double>{};
    }
    if (num == 1)
    {
        return std::vector<double>{start};
    }

    double step = (end - start) / (num - 1);

    auto result = std::views::iota(0, num)
                  | std::views::transform([=](int i) { return start + i * step; });

    return std::vector<double>(result.begin(), result.end());
}

/// @brief Compute the Bellman values
/// @return The bellman values for each week
std::vector<std::vector<double>> GridEvaluator::ComputeBellmanValues(int startWeek, int endWeek)
{
    // auto X = gridDefinition.gridElements[0].values; // discretization of hydro level
    auto X = linspace(0.0, reservoirManagement.reservoir.capacity, 11);
    std::map<ScenarioAndWeek, std::vector<double>> V;
    std::map<ScenarioAndWeek, std::vector<double>> rewards;

    for (const auto& [key, reward]: variationDeNiveauxDeStockData.get())
    {
        rewards[{key.scenario, key.week}].push_back(reward);
    }

    for (int week = startWeek; week <= endWeek + 1; ++week)
    {
        for (int scenario = 1; scenario <= nbScenarios; ++scenario)
        {
            V[{scenario, week}] = std::vector<double>(X.size(), 0.0);
        }
    }

    for (int week = endWeek; week >= startWeek; --week)
    {
        for (int scenario = 1; scenario <= nbScenarios; ++scenario)
        {
            auto V_fut = [&X, &V, &week, &scenario]()
            {
                auto& V_vec = V[{scenario, week + 1}];
                return [&X, &V_vec, &week, &scenario](double x)
                { return interpolate(X, V_vec, x); };
            };

            std::set<double> breaking_points = {
              *gridDefinition.gridElements[0].values[{scenario, week}].begin(),
              *gridDefinition.gridElements[0].values[{scenario, week}].rbegin()};
            for (size_t i = 0; i < X.size(); ++i)
            {
                double Vu = SolveWeeklyProblemWithReward(week,
                                                         scenario,
                                                         X[i],
                                                         X,
                                                         rewards[{scenario, week}],
                                                         V_fut(),
                                                         breaking_points);
                V[{scenario, week}][i] += Vu;
            }
        }

        for (int i = 0; i < X.size(); ++i)
        {
            double sum = 0.0;
            for (int scenario = 1; scenario <= nbScenarios; ++scenario)
            {
                sum += V[{scenario, week}][i];
            }
            for (int scenario = 1; scenario <= nbScenarios; ++scenario)
            {
                V[{scenario, week}][i] = sum / nbScenarios;
            }
        }
    }

    std::vector<std::vector<double>> V_final;
    for (int week = startWeek; week <= endWeek; ++week)
    {
        V_final.push_back(V[{1, week}]);
    }

    return V_final;
}

/// @brief Solve the weekly problem
/// @param week
/// @param scenario
/// @param level the level of the reservoir
/// @param X the discretization of the reservoir level
/// @param rewards the rewards for each scenario and week
/// @param V_fut the reward function for the next week
/// @param breaking_points
/// @return The Bellvalue for a given week and scenario and level of the reservoir
double GridEvaluator::SolveWeeklyProblemWithReward(int week,
                                                   int scenario,
                                                   double level,
                                                   const std::vector<double>& X,
                                                   const std::vector<double>& rewards,
                                                   const std::function<double(double)>& V_fut,
                                                   std::set<double>& breaking_points)
{
    double Vu = std::numeric_limits<double>::max();
    const Reservoir reservoir = reservoirManagement.reservoir;

    for (double value_fut: X)
    {
        double u = -value_fut + level + reservoir.inflow[week - 1][scenario - 1];
        if ((-reservoir.max_pumping[week - 1] * reservoir.efficiency <= u)
            && (reservoirManagement.overflow || u <= reservoir.max_generating[week - 1]))
        {
            u = std::min(u, reservoir.max_generating[week - 1]);
            double G = interpolate(gridDefinition.gridElements[0].values[{scenario, week}],
                                   rewards,
                                   u);
            if (G + V_fut(value_fut) < Vu)
            {
                Vu = G + V_fut(value_fut);
            }
        }
    }

    for (double u: gridDefinition.gridElements[0].values[{scenario, week}])
    {
        double state_fut = level - u + reservoir.inflow[week - 1][scenario - 1];
        if (0 <= state_fut && state_fut <= reservoir.capacity)
        {
            double G = interpolate(gridDefinition.gridElements[0].values[{scenario, week}],
                                   rewards,
                                   u);
            if (G + V_fut(state_fut) < Vu)
            {
                Vu = G + V_fut(state_fut);
            }
        }
    }

    return Vu;
}
