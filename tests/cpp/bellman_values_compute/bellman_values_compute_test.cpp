#include <algorithm>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
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
        std::filesystem::path data_dir = "data_test/one_node_base";
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
};

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

    auto valeurs_usage = GridEvaluator(logger,
                                       writer,
                                       mps_path,
                                       grid,
                                       reservoir_management,
                                       ProblemsFormat::MPS_FILE,
                                       8);
    valeurs_usage.ComputeRewards();
    auto res = valeurs_usage.ComputeBellmanValues();

    for (int week = 1; week <= res.size(); week++)
    {
        for (int level_index = 0; level_index < res[week].size(); level_index++)
        {
            double cost = res[week - 1][level_index];
            double expected_cost = expected_costs[{1, week}][0];
            EXPECT_NEAR_REL(cost, expected_cost, 1e-6);
            expected_costs[{1, week}].erase(expected_costs[{1, week}].begin());
        }
    }
}
