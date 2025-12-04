#include <algorithm>

#include "RandomDirGenerator.h"
#include "antares-xpansion/bellman_values/BellmanValues.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/logger/Master.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/lpnamer/main/ConfigurationManager.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

#define EXPECT_NEAR_REL(val1, val2, rel_tol)                                                       \
    EXPECT_TRUE(std::abs((val1) - (val2)) <= (rel_tol) * std::abs(val2))                           \
      << "Expected: " << (val2) << ", Actual: " << (val1) << ", Relative tolerance: " << (rel_tol) \
      << ", Relative error: " << std::abs((val1) - (val2)) / std::abs(val2)

class BellmanValuesComputeTest: public ::testing::Test
{
public:
    Logger logger;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver;
    std::filesystem::path tmpDir;
    const std::filesystem::path data_test_dir = "data_test";
    const std::string solverName = "xpress";

protected:
    void SetUp() override
    {
        // adding a new logger that actually logs
        Logger std_out_logger;
        std_out_logger = std::make_shared<xpansion::logger::User>(std::cerr);
        auto master_logger = std::make_shared<xpansion::logger::Master>();
        master_logger->addLogger(std_out_logger);
        logger = master_logger;
        original_dir = std::filesystem::current_path();
    }

    void TearDown() override
    {
        std::filesystem::current_path(original_dir);
    }

    void copyData()
    {
        std::filesystem::path data_dir = data_test_dir / "one_node_base";
        tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());

        std::filesystem::copy(data_dir,
                              tmpDir,
                              std::filesystem::copy_options::recursive
                                | std::filesystem::copy_options::update_existing);
    }

    std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> getOutputCosts(
      std::string fileName)
    {
        std::map<Antares::Solver::WeeklyProblemId, std::vector<double>> costs;
        std::ifstream file(tmpDir / fileName);
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;
            unsigned int week = 1;
            unsigned int scenario = 1;
            while (std::getline(ss, token, ','))
            {
                costs[{scenario, week}].push_back(std::stod(token));
                week++;
            }
        }
        return costs;
    }

    std::filesystem::path original_dir;

    struct GridEvaluatorMock: public GridEvaluator
    {
        GridEvaluatorMock():
            GridEvaluator(nullptr, {}, gridDef, "mockSolver", 0)
        {
        }

        std::map<Output::PointWeekScenarioKey, GridPointResult> ComputeCostsAndDuals() override
        {
            return {
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 1, 1), {100.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 2, 1), {100.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 3, 1), {100.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 1, 1), {80.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 2, 1), {80.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 3, 1), {80.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 1, 1), {60.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 2, 1), {60.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 3, 1), {60.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 1, 1), {40.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 2, 1), {40.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 3, 1), {40.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 1, 1), {20.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 2, 1), {20.0}},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 3, 1), {20.0}},
            };
        }

        Reservoir reservoirMock = Reservoir("TestArea",
                                            500.0,
                                            1.0,
                                            0,
                                            {100.0, 100.0, 100.0},             // max_generating
                                            {100.0, 100.0, 100.0},             // max_pumping
                                            {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, // inflow
                                            {100, 100, 100, 100},              // bottom rule curve
                                            {400, 400, 400, 400}               // top rule curve
        );

    private:
        std::map<std::string, Reservoir> reservoirs{{"area", reservoirMock}};
        std::vector<std::vector<double>> values = {
          {-100, -50, 0, 50, 100},
          {-100, -50, 0, 50, 100},
          {-100, -50, 0, 50, 100},
        };

        GridDefinition gridDef = {.gridID = 0,
                                  .reservoirs = reservoirs,
                                  .gridElements = {{
                                    .name = "cst",
                                    .area = "area",
                                    .rhsValues = values,
                                  }},
                                  .weekAreaConstraints = {
                                    {1, {{"area", {{"cst", values[0]}}}}},
                                    {2, {{"area", {{"cst", values[1]}}}}},
                                    {3, {{"area", {{"cst", values[2]}}}}},
                                  }};
    } evaluatorMock;
};

TEST_F(BellmanValuesComputeTest, unitTestNoPenalties)
{
    ReservoirManagement reservoirManagement(evaluatorMock.reservoirMock, 0, 0, 0);
    auto bellmanValues = BellmanValues(evaluatorMock, reservoirManagement).compute(6);

    std::vector<std::vector<double>> expected = {{180, 140, 100, 60, 60, 60},
                                                 {120, 80, 40, 40, 40, 40},
                                                 {60, 20, 20, 20, 20, 20},
                                                 {0, 0, 0, 0, 0, 0}};

    EXPECT_EQ(bellmanValues, expected);
}

TEST_F(BellmanValuesComputeTest, unitTestPenalties)
{
    ReservoirManagement reservoirManagement(evaluatorMock.reservoirMock, 10, 10, 0);
    auto bellmanValues = BellmanValues(evaluatorMock, reservoirManagement).compute(6);

    std::vector<std::vector<double>> expected = {{1220, 180, 140, 100, 60, 1060},
                                                 {1160, 120, 80, 40, 40, 1040},
                                                 {1100, 60, 20, 20, 20, 1020},
                                                 {1000, 0, 0, 0, 0, 1000}};

    EXPECT_EQ(bellmanValues, expected);
}

TEST_F(BellmanValuesComputeTest, unitTestPenaltiesWithFinalLevel)
{
    ReservoirManagement reservoirManagement(evaluatorMock.reservoirMock, 10, 10, 30, true, 400);
    auto bellmanValues = BellmanValues(evaluatorMock, reservoirManagement).compute(6);

    std::vector<std::vector<double>> expected = {{4300, 300, 260, 220, 180, 1140},
                                                 {7200, 3200, 200, 160, 120, 1080},
                                                 {10100, 6100, 3100, 100, 60, 1020},
                                                 {12000, 9000, 6000, 3000, 0, 3000}};

    EXPECT_EQ(bellmanValues, expected);
}

TEST_F(BellmanValuesComputeTest, OneNodeBaseCaseNoPenalties)
{
    copyData();
    auto expected_costs = getOutputCosts("result_bellman_values_no_penalties.csv");

    auto grid_collection = GridCollection(tmpDir / "grid.csv");
    auto grid = grid_collection.gridDefinitions.at(0);
    ReservoirManagement reservoir_management(grid_collection.reservoirs.begin()->second, 0, 0, 0);

    ConfigurationManager::ConfigDirectories config_dirs{
      .study_dir = tmpDir,
      .simulation_dir = ConfigurationManager::generateOutputName(tmpDir),
    };

    ProblemGenerationForWaterValueCalculation pbg(config_dirs, logger, solverName);
    auto problems = pbg.updateProblems(grid, reservoir_management);

    auto evaluator = GridEvaluator(logger, problems, grid, solverName, 8);
    auto res = BellmanValues(evaluator, reservoir_management).compute(11);

    for (unsigned int week = 1; week < res.size(); week++)
    {
        for (int level_index = 0; level_index < res[week - 1].size(); level_index++)
        {
            double cost = res[week - 1][level_index];
            double expected_cost = expected_costs[{1, week}][0];
            EXPECT_NEAR_REL(cost, expected_cost, 1e-6);
            expected_costs[{1, week}].erase(expected_costs[{1, week}].begin());
        }
    }
}

TEST_F(BellmanValuesComputeTest, OneNodeBaseCasePenalties)
{
    copyData();
    auto expected_costs = getOutputCosts("result_bellman_values_penalties.csv");

    auto grid_collection = GridCollection(tmpDir / "grid.csv");
    auto grid = grid_collection.gridDefinitions.at(0);
    ReservoirManagement reservoir_management(grid_collection.reservoirs.begin()->second,
                                             3000,
                                             3000,
                                             3000);

    ConfigurationManager::ConfigDirectories config_dirs{
      .study_dir = tmpDir,
      .simulation_dir = ConfigurationManager::generateOutputName(tmpDir),
    };

    ProblemGenerationForWaterValueCalculation pbg(config_dirs, logger, solverName);
    auto problems = pbg.updateProblems(grid, reservoir_management);

    auto evaluator = GridEvaluator(logger, problems, grid, solverName, 8);
    auto res = BellmanValues(evaluator, reservoir_management).compute(11);

    for (unsigned int week = 1; week < res.size(); week++)
    {
        for (int level_index = 0; level_index < res[week - 1].size(); level_index++)
        {
            double cost = res[week - 1][level_index];
            double expected_cost = expected_costs[{1, week}][0];
            EXPECT_NEAR_REL(cost, expected_cost, 1e-6);
            expected_costs[{1, week}].erase(expected_costs[{1, week}].begin());
        }
    }
}

TEST_F(BellmanValuesComputeTest, OneNodeBaseCasePenaltiesWithFinalLevel)
{
    copyData();
    auto expected_costs = getOutputCosts("result_bellman_values_penalties_final_level.csv");

    auto grid_collection = GridCollection(tmpDir / "grid.csv");
    auto grid = grid_collection.gridDefinitions.at(0);
    ReservoirManagement reservoir_management(grid_collection.reservoirs.begin()->second,
                                             3000,
                                             3000,
                                             3000,
                                             true);

    ConfigurationManager::ConfigDirectories config_dirs{
      .study_dir = tmpDir,
      .simulation_dir = ConfigurationManager::generateOutputName(tmpDir),
    };

    ProblemGenerationForWaterValueCalculation pbg(config_dirs, logger, solverName);
    auto problems = pbg.updateProblems(grid, reservoir_management);

    auto evaluator = GridEvaluator(logger, problems, grid, solverName, 8);
    auto res = BellmanValues(evaluator, reservoir_management).compute(11);

    for (unsigned int week = 1; week < res.size(); week++)
    {
        for (int level_index = 0; level_index < res[week - 1].size(); level_index++)
        {
            double cost = res[week - 1][level_index];
            double expected_cost = expected_costs[{1, week}][0];
            EXPECT_NEAR_REL(cost, expected_cost, 1e-6);
            expected_costs[{1, week}].erase(expected_costs[{1, week}].begin());
        }
    }
}
