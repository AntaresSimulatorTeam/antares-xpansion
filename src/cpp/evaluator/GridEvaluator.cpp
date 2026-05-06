
#include "antares-xpansion/evaluator/GridEvaluator.h"

#include <fmt/core.h>
#include <regex>
#include <sstream>
#include <tbb/global_control.h>
#include <tbb/parallel_for_each.h>
#include <utility>

#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/helpers/Timer.h"

using namespace PlainData;

/// @brief Constructor
/// @param logger Logger
/// @param problems map of subproblems to evaluate
/// @param gridDefinition GridCollection containing the grids to evaluate
/// @param studyDir Path to the study, used to save MPS files in case of error
/// @param solverName Name of the solver to use
/// @param nbThreads Number of threads to use
GridEvaluator::GridEvaluator(
  Logger logger,
  std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems,
  GridDefinition& gridDefinition,
  std::filesystem::path studyDir,
  std::string solverName,
  int nbThreads):
    Evaluator(logger, problems, studyDir, solverName, nbThreads),
    gridDefinition(gridDefinition)
{
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
/// @param problemId Id of the subproblem (used to prefix constraint names).
/// @param areasConstraints Map from area names to their constraint maps.
/// @return A vector of maps, where each map is a full combination of all constraints
///         (with area-prefixed keys) for the subproblem.
ConstraintCombos GridEvaluator::GenerateSubPbCombos(
  const Antares::Solver::WeeklyProblemId problemId,
  const AreaConstraintMaps& areasConstraints)
{
    ConstraintCombos subPbCombos;
    ConstraintCombos currentCombos = {{}};

    for (const auto& [areaName, constraints]: areasConstraints)
    {
        logger->display_message("Processing areasConstraints for area " + areaName,
                                LogUtils::LOGLEVEL::DEBUG,
                                GRID_EVALUATOR_LOGGER_CONTEXT);
        ConstraintCombos newCombos;
        ConstraintCombos localCombos = GenerateConstraintProduct(constraints);
        logger->display_message("localCombos size: " + std::to_string(localCombos.size())
                                  + " for area " + areaName,
                                LogUtils::LOGLEVEL::DEBUG,
                                GRID_EVALUATOR_LOGGER_CONTEXT);

        for (const auto& combo: currentCombos)
        {
            for (const auto& local: localCombos)
            {
                std::map<std::string, double> merged = combo;
                for (const auto& [cst, val]: local)
                {
                    logger->display_message("Adding value " + std::to_string(val)
                                              + " to constraint "
                                              + GetConstraintName(problemId, areaName, cst),
                                            LogUtils::LOGLEVEL::DEBUG,
                                            GRID_EVALUATOR_LOGGER_CONTEXT);
                    merged[GetConstraintName(problemId, areaName, cst)] = val;
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

/// @brief Process a single subproblem
/// @details this function generates all possible combinations of right-hand side (RHS)
/// constraint
///          values using `GenerateSubPbCombos`, applies them to the model via
///          `SetConstraintsRHSValues`, solves the subproblem using `SolveSubproblem`, and
///          stores the resulting cost in the `variationDeNiveauxDeStockResults` map indexed by
///          scenario, week, and constraint values.
/// @param subProblemId the id of the problem to treat
/// @param subPronlem the problem to treat
void GridEvaluator::ProcessSubproblem(const Antares::Solver::WeeklyProblemId subProblemId,
                                      std::shared_ptr<Problem> subProblem)
{
    std::vector<int> dims;

    for (const auto& [area, constraints]: gridDefinition.weekAreaConstraints.at(subProblemId.week))
    {
        std::transform(constraints.begin(),
                       constraints.end(),
                       std::back_inserter(dims),
                       [](const auto& constraints) { return constraints.second.size(); });
    }

    ConstraintCombos subPbCombos = GenerateSubPbCombos(subProblemId,
                                                       gridDefinition.weekAreaConstraints.at(
                                                         subProblemId.week));
    subPbCombos = reorderZigzagND(dims, subPbCombos);

    int size = subPbCombos.size();
    int i = 0;
    for (const auto& subPbCombo: subPbCombos)
    {
        i++;
        logger->display_message(
          (std::stringstream() << "Processing gridPoint " << i << "/" << size).str(),
          LogUtils::LOGLEVEL::DEBUG,
          GRID_EVALUATOR_LOGGER_CONTEXT);
        // Each areaCombo is a std::map<std::string, double> with full variable names
        Timer timer;
        SetConstraintsRHSValues(subPbCombo, subProblem);
        totalPbModifTimer += timer.elapsed();
        SubProblemData res = SolveSubproblem(subProblem);

        std::vector<double> dualValuesCst(subProblem->get_nrows());
        subProblem->get_lp_sol(NULL, dualValuesCst.data(), NULL);
        for (const auto& [constraintName, value]: subPbCombo)
        {
            res.dual.emplace(constraintName,
                             dualValuesCst[subProblem->get_row_index(constraintName)]);
        }

        variationDeNiveauxDeStockResults.insert({subPbCombo, subProblemId.week, subProblemId.year},
                                                res);
        logger->display_message((std::stringstream() << "Cost: " << res.subproblem_cost).str(),
                                LogUtils::LOGLEVEL::DEBUG,
                                GRID_EVALUATOR_LOGGER_CONTEXT);
    }
}

/// @brief Launch the Stock level variation computation
/// @return The stock level variation results
std::map<Output::PointWeekScenarioKey, SubProblemData> GridEvaluator::ComputeCostsAndDuals()
{
    logger->display_message((std::stringstream() << "Launching Stock level variation").str(),
                            LogUtils::LOGLEVEL::INFO,
                            GRID_EVALUATOR_LOGGER_CONTEXT);

    // Time the Run time
    Timer run_timer;

    Run();

    auto run_time = run_timer.elapsed();
    logger->display_message((std::stringstream()
                             << "Stock level variation done in " << run_time << " seconds and "
                             << totalSimplexIter << " simplex iterations")
                              .str(),
                            LogUtils::LOGLEVEL::INFO,
                            GRID_EVALUATOR_LOGGER_CONTEXT);
    logger->display_message((std::stringstream()
                             << "Time solving subproblems (accumulated by each thread) : "
                             << totalSubPbTimer << " seconds")
                              .str(),
                            LogUtils::LOGLEVEL::INFO,
                            GRID_EVALUATOR_LOGGER_CONTEXT);

    return variationDeNiveauxDeStockResults.get();
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
