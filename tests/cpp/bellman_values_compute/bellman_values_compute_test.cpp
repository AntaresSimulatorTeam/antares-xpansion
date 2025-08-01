#include <algorithm>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/bellman_values/BellmanValues.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
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
    std::shared_ptr<Output::JsonWriter> writer;
    std::filesystem::path tmpDir;
    const std::filesystem::path data_test_dir = "data_test";

protected:
    void SetUp() override
    {
        logger = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                      std::tmpnam(nullptr));
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

    std::map<ScenarioAndWeek, std::vector<double>> getOutputCosts()
    {
        std::map<ScenarioAndWeek, std::vector<double>> costs;

        std::ifstream file(tmpDir / "result_bellman_values.csv");
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;
            int week = 1;
            int scenario = 1;
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
            GridEvaluator(nullptr,
                          nullptr,
                          "mockPath",
                          gridDef,
                          ProblemsFormat::MPS_FILE,
                          "mockSolver",
                          0)
        {
        }

        std::map<Output::PointWeekScenarioKey, double> ComputeRewards(int startWeek,
                                                                      int endWeek) override
        {
            return {
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 1, 1), 20.0},
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 2, 1), 20.0},
              {Output::PointWeekScenarioKey({{"HydroPower", -100}}, 3, 1), 20.0},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 1, 1), 40.0},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 2, 1), 40.0},
              {Output::PointWeekScenarioKey({{"HydroPower", -50}}, 3, 1), 40.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 1, 1), 60.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 2, 1), 60.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 0}}, 3, 1), 60.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 1, 1), 80.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 2, 1), 80.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 50}}, 3, 1), 80.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 1, 1), 100.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 2, 1), 100.0},
              {Output::PointWeekScenarioKey({{"HydroPower", 100}}, 3, 1), 100.0},
            };
        }

    private:
        GridDefinition gridDef = {.gridID = 0,
                                  .gridElements = {{.values = {
                                                      {{1, 1}, {-100, -50, 0, 50, 100}},
                                                      {{1, 2}, {-100, -50, 0, 50, 100}},
                                                      {{1, 3}, {-100, -50, 0, 50, 100}},
                                                    }}}};
    } evaluatorMock;

    Reservoir createTestReservoir()
    {
        return Reservoir("TestArea",
                         500.0,
                         1.0,
                         {100.0, 100.0, 100.0},            // max_generating
                         {100.0, 100.0, 100.0},            // max_pumping
                         {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}} // inflow
        );
    }
};

TEST_F(BellmanValuesComputeTest, unitTest)
{
    Reservoir reservoir = createTestReservoir();
    ReservoirManagement reservoirManagement(reservoir, false);
    auto bellmanValues = BellmanValues(evaluatorMock, reservoirManagement).compute(1, 3, 6);

    std::vector<std::vector<double>> expected = {{60, 60, 60, 100, 140, 180},
                                                 {40, 40, 40, 40, 80, 120},
                                                 {20, 20, 20, 20, 20, 60}};

    EXPECT_EQ(bellmanValues, expected);
}

TEST_F(BellmanValuesComputeTest, OneNodeBaseCase)
{
    copyData();
    auto expected_costs = getOutputCosts();

    auto grid_collection = GridCollection(tmpDir / "grid.csv");
    auto grid = grid_collection.gridDefinitions[0];
    Reservoir reservoir(tmpDir, "area");
    ReservoirManagement reservoir_management(reservoir, true);

    auto options_parser = ProblemGenerationExeOptions();
    auto config_manager = ConfigurationManager(options_parser);

    ConfigurationManager::ConfigDirectories config_dirs{
      .study_dir = tmpDir,
      .simulation_dir = config_manager.generateOutputName(tmpDir),
    };

    ProblemGenerationForWaterValueCalculation pbg(config_dirs);
    auto mps_path = pbg.updateProblems(grid);

    auto evaluator = GridEvaluator(logger,
                                   writer,
                                   mps_path,
                                   grid,
                                   ProblemsFormat::MPS_FILE,
                                   "XPRESS",
                                   8);
    auto res = BellmanValues(evaluator, reservoir_management).compute(1, 52, 11);

    for (int week = 1; week < res.size(); week++)
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
