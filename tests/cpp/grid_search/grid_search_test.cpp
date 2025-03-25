#include <algorithm>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/BendersMathLogger.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_search/GridSearch.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

class GridSearchTest: public ::testing::Test
{
public:
    Logger logger;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver;
    std::shared_ptr<Output::OutputWriter> writer;
    const std::filesystem::path data_test_dir = "data_test";
    const std::filesystem::path mps_dir = data_test_dir / "mps";
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
        std::filesystem::path data_test_dir = "data_test";
        std::filesystem::path data_dir = data_test_dir / "mini_instance_LP";
        tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());

        std::filesystem::copy(data_dir,
                              tmpDir,
                              std::filesystem::copy_options::recursive
                                | std::filesystem::copy_options::update_existing);
    }

    std::vector<double> getOutputCosts()
    {
        std::vector<double> output_costs;
        std::ifstream file(tmpDir / "output_cost.csv");
        std::string line;
        std::getline(file, line);

        while (std::getline(file, line))
        {
            // Get the last element of the line (separated by commas)
            size_t last_comma_pos = line.find_last_of(',');
            std::string last_token = line.substr(last_comma_pos + 1);
            output_costs.push_back(std::stod(last_token));
        }
        return output_costs;
    }

    std::filesystem::path original_dir;
};

TEST_F(GridSearchTest, MiniInstanceLP)
{
    copyData();

    auto grid_search = GridSearch(logger, writer, tmpDir);

    grid_search.launch();

    auto output_costs = getOutputCosts();
    for (size_t i = 0; i < grid_search.gridPointData.size(); i++)
    {
        EXPECT_NEAR(grid_search.gridPointData[i].overall_cost, output_costs[i], 1e-6);
    }
}

// Problems svf
