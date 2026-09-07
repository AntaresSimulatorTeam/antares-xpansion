#include "antares-xpansion/benders/benders_core/CutsManager.hxx"
#include "gtest/gtest.h"

// Minimal CutsManager test double — only ComputeXCut is under test,
// so GatherAndBuildCutsImpl is never called.
class CutsManagerTestDouble: public CutsManager<CutsManagerTestDouble>
{
public:
    void GatherAndBuildCutsImpl()
    {
    }
};

TEST(ComputeXCutTest, FirstIteration)
{
    double sep_param = 0.8;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    data.max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};
    data.it = 1;

    CutsManagerTestDouble cuts_manager;
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // In first iteration, x_cut should equal x_out
    EXPECT_EQ(data.x_cut, data.x_out);
}

TEST(ComputeXCutTest, LaterIteration)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.min_invest = {{"x1", -1e+20}, {"x2", -1e+20}};
    data.max_invest = {{"x1", 1e+20}, {"x2", 1e+20}};
    data.it = 2;

    CutsManagerTestDouble cuts_manager;
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x_cut = sep_param * x_out + (1 - sep_param) * x_in, no rounding here
    Point expected_x_cut = {{"x1", 2.0}, {"x2", 4.0}};
    EXPECT_EQ(data.x_cut, expected_x_cut);
}

TEST(ComputeXCutTest, RoundingLowerBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.x_out = {{"x1", 1.0}, {"x2", 2.0}};
    data.x_in = {{"x1", 1.01}, {"x2", 6.0}};
    data.min_invest = {{"x1", 1}, {"x2", -1e+20}};
    data.max_invest = {{"x1", 10}, {"x2", 1e+20}};
    data.it = 2;

    CutsManagerTestDouble cuts_manager;
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x1 rounded to lower bound (1.005 without rounding)
    Point expected_x_cut{{"x1", 1.0}, {"x2", 4.0}};
    EXPECT_EQ(data.x_cut, expected_x_cut);
}

TEST(ComputeXCutTest, RoundingUpperBound)
{
    double sep_param = 0.5;
    double master_solution_tolerance = 0.1;

    CurrentIterationData data;
    data.x_out = {{"x1", 1.0}, {"x2", 5.99}};
    data.x_in = {{"x1", 3.0}, {"x2", 6.0}};
    data.min_invest = {{"x1", 1}, {"x2", 0}};
    data.max_invest = {{"x1", 10}, {"x2", 6.0}};
    data.it = 2;

    CutsManagerTestDouble cuts_manager;
    cuts_manager.ComputeXCut(data, sep_param, master_solution_tolerance);

    // x1 not rounded, x2 rounded to upper bound (5.995 without rounding)
    Point expected_x_cut{{"x1", 2.0}, {"x2", 6.0}};
    EXPECT_EQ(data.x_cut, expected_x_cut);
}
