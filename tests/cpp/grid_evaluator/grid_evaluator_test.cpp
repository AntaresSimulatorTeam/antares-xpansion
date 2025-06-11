#include <algorithm>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

#define EXPECT_NEAR_REL(val1, val2, rel_tol)                                                       \
    EXPECT_TRUE(std::abs((val1) - (val2)) <= (rel_tol) * std::abs(val2))                           \
      << "Expected: " << (val2) << ", Actual: " << (val1) << ", Relative tolerance: " << (rel_tol) \
      << ", Relative error: " << std::abs((val1) - (val2)) / std::abs(val2)

class GridSearchTest: public ::testing::Test
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
        std::filesystem::path data_dir = "data_test/mps_use_case_vu";
        tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());

        std::filesystem::copy(data_dir,
                              tmpDir,
                              std::filesystem::copy_options::recursive
                                | std::filesystem::copy_options::update_existing);
    }

    std::map<ScenarioAndWeek, std::vector<double>> getOutputCosts()
    {
        std::map<ScenarioAndWeek, std::vector<double>> costs;

        std::ifstream file(tmpDir / "result.csv");
        std::string line;
        std::getline(file, line);

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(ss, token, ','))
            {
                tokens.push_back(token);
            }
            int scenario = std::stoi(tokens[4]);
            int week = std::stoi(tokens[1]);
            double cost = std::stod(tokens[5]);

            costs[{scenario + 1, week + 1}].push_back(cost);
        }
        return costs;
    }

    std::filesystem::path original_dir;
};

TEST_F(GridSearchTest, MPSUseCaseValeursUsage)
{
    copyData();

    auto valeurs_usage = GridEvaluator(logger, writer, tmpDir, ProblemsFormat::MPS_FILE, 8);
    valeurs_usage.launch();

    auto output_costs = getOutputCosts();
    EXPECT_EQ(output_costs.size(), 52 * 10);
    for (const auto& [key, cost]: valeurs_usage.variationDeNiveauxDeStockData)
    {
        ScenarioAndWeek keyStruct{key.scenario, key.week};

        if (output_costs.count(keyStruct) > 0 && !output_costs[keyStruct].empty())
        {
            EXPECT_NEAR_REL(output_costs[keyStruct][0], cost, 1e-6);
            output_costs[keyStruct].erase(output_costs[keyStruct].begin());
        }
        else
        {
            FAIL() << "Missing or empty entry for key: " << keyStruct.scenario << ", "
                   << keyStruct.week;
        }
    }
}
