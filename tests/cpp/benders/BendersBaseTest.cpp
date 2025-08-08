#include <algorithm>
#include <antares-xpansion/benders/benders_core/SimulationOptions.h>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/helpers/ArchiveWriter.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

// Test double class for BendersBase to expose protected members and override virtual methods
class BendersBaseDouble: public BendersBase
{
public:
    explicit BendersBaseDouble(const BendersBaseOptions& options,
                               Logger& logger,
                               std::shared_ptr<Output::OutputWriter> writer,
                               std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
        BendersBase(options, logger, writer, mathLoggerDriver)
    {
    }

    // Expose protected methods for testing
    using BendersBase::_data;
    using BendersBase::ComputeXCut;

    // Override pure virtual methods
    void free() override
    {
    }

    void launch() override
    {
    }

    std::string BendersName() const override
    {
        return "";
    }

    void InitializeProblems() override
    {
    }

    void Run() override
    {
    }

    void get_master_value() override
    {
    }

    void init_data() override
    {
        BendersBase::init_data();
    }

    void UpdateTrace() override
    {
    }

    void post_run_actions() const override
    {
    }

    void EndWritingInOutputFile() const override
    {
    }

    void SaveCurrentBendersData() override
    {
    }

    void write_basis() const override
    {
    }

    // Helper methods for testing
    void set_data(Point x_out, Point x_in)
    {
        _data.x_out = x_out;
        _data.x_in = x_in;
    }

    // Helper methods for testing
    void set_invest_bounds(Point min_invest, Point max_invest)
    {
        _data.min_invest = min_invest;
        _data.max_invest = max_invest;
    }

    void set_iteration(int it)
    {
        _data.it = it;
    }

    Point get_x_cut() const
    {
        return _data.x_cut;
    }

protected:
    [[nodiscard]] bool shouldParallelize() const final
    {
        return false;
    }
};

class BendersBaseTest: public ::testing::Test
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

    BendersBaseOptions init_benders_options(double sep_param,
                                            double master_solution_tolerance) const
    {
        SolverBaseOptions solver_options;
        solver_options.LOG_LEVEL = 0;
        solver_options.SLAVE_WEIGHT_VALUE = 1;
        solver_options.OUTPUTROOT = "my_output";
        solver_options.SLAVE_WEIGHT = "CONSTANT";
        solver_options.MASTER_NAME = "master";
        solver_options.STRUCTURE_FILE = "my_structure.txt";
        solver_options.SOLVER_NAME = "COIN";
        solver_options.weights = {};

        BendersBaseOptions options(solver_options);
        options.SEPARATION_PARAM = sep_param;
        options.MASTER_FORMULATION = MasterFormulation::RELAXED;
        options.RESUME = false;
        options.AGGREGATION = false;
        options.TRACE = false;
        options.BOUND_ALPHA = true;
        options.MASTER_SOLUTION_TOLERANCE = 0.1;

        return options;
    }

    std::filesystem::path original_dir;
};

TEST_F(BendersBaseTest, ComputeXCutFirstIteration)
{
    double sep_param = 0.8;
    double master_solution_tolerance = 0.1;
    BendersBaseDouble benders = BendersBaseDouble(init_benders_options(sep_param,
                                                                       master_solution_tolerance),
                                                  logger,
                                                  writer,
                                                  mathLoggerDriver);

    Point x_out = {{"x1", 1.0}, {"x2", 2.0}};
    Point x_in = {{"x1", 3.0}, {"x2", 6.0}};
    Point min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    Point max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};

    benders.set_data(x_out, x_in);
    benders.set_invest_bounds(min_invest, max_invest);
    benders.set_iteration(1);

    benders.ComputeXCut();
    Point x_cut = benders.get_x_cut();

    // In first iteration, x_cut should equal x_out
    EXPECT_EQ(x_cut, x_out);
}

TEST_F(BendersBaseTest, ComputeXCutLaterIteration)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;
    BendersBaseDouble benders = BendersBaseDouble(init_benders_options(sep_param,
                                                                       master_solution_tolerance),
                                                  logger,
                                                  writer,
                                                  mathLoggerDriver);

    Point x_out = {{"x1", 1.0}, {"x2", 2.0}};
    Point x_in = {{"x1", 3.0}, {"x2", 6.0}};
    Point min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    Point max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};

    benders.set_data(x_out, x_in);
    benders.set_invest_bounds(min_invest, max_invest);
    benders.set_iteration(2);

    benders.ComputeXCut();
    Point x_cut = benders.get_x_cut();

    // In later iterations, x_cut should be a convex combination
    // x_cut = sep_param * x_out + (1 - sep_param) * x_in, no rounding here
    Point expected_x_cut = {{"x1", 2.0}, {"x2", 4.0}};
    ;

    EXPECT_EQ(x_cut, expected_x_cut);
}

TEST_F(BendersBaseTest, ComputeXCutWithRoundingLowerBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;
    BendersBaseDouble benders = BendersBaseDouble(init_benders_options(sep_param,
                                                                       master_solution_tolerance),
                                                  logger,
                                                  writer,
                                                  mathLoggerDriver);

    Point x_out = {{"x1", 1.0}, {"x2", 2.0}};
    Point x_in = {{"x1", 1.01}, {"x2", 6.0}};
    Point min_invest = {{"x1", 1}, {"x2", -1e+20}};
    Point max_invest = {{"x1", 10}, {"x2", 1e+20}};

    benders.set_data(x_out, x_in);
    benders.set_invest_bounds(min_invest, max_invest);
    benders.set_iteration(2);

    benders.ComputeXCut();
    Point x_cut = benders.get_x_cut();

    // x1 rounded to lower bound (1.005 without rounding)
    Point expected_x_cut{{"x1", 1.0}, {"x2", 4.0}};

    EXPECT_EQ(x_cut, expected_x_cut);
}

TEST_F(BendersBaseTest, ComputeXCutWithRoundingupperBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;
    BendersBaseDouble benders = BendersBaseDouble(init_benders_options(sep_param,
                                                                       master_solution_tolerance),
                                                  logger,
                                                  writer,
                                                  mathLoggerDriver);

    Point x_out = {{"x1", 1.0}, {"x2", 5.99}};
    Point x_in = {{"x1", 3.0}, {"x2", 6.0}};
    Point min_invest = {{"x1", 1}, {"x2", 0}};
    Point max_invest = {{"x1", 10}, {"x2", 6.0}};

    benders.set_data(x_out, x_in);
    benders.set_invest_bounds(min_invest, max_invest);
    benders.set_iteration(2);

    benders.ComputeXCut();
    Point x_cut = benders.get_x_cut();

    // x1 not rounded, x2 rounded to upper bound (5.995 without rounding)
    Point expected_x_cut{{"x1", 2.0}, {"x2", 6.0}};

    EXPECT_EQ(x_cut, expected_x_cut);
}
