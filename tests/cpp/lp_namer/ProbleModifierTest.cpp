
#include <ProblemModifier.h>
#include <multisolver_interface/SolverFactory.h>
#include <fstream>
#include "gtest/gtest.h"


class ProblemModifierTest : public ::testing::Test
{
public:
    const std::string mps_name = "dummy.mps";
    SolverAbstract::Ptr math_problem;

protected:
    void SetUp()
    {
        std::string mps_content= "NAME          Pb Solve\n"
                                 "ROWS\n"
                                 " N  OBJECTIF\n"
                                 "COLUMNS\n"
                                 "    C0000000  OBJECTIF  1.0000000000\n"
                                 "RHS\n"
                                 "BOUNDS\n"
                                 " LO BNDVALUE  C0000000  -1000.000000000\n"
                                 " UP BNDVALUE  C0000000  1000.000000000\n"
                                 "ENDATA";
        std::ofstream file_mps(mps_name.c_str());
        file_mps << mps_content;
        file_mps.close();
        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        math_problem->read_prob_mps(mps_name);
        std::remove(mps_name.c_str());
    }

    void TearDown()
    {
        // tear down
    }
};

TEST_F(ProblemModifierTest, empty_test)
{
    int n_cols = math_problem->get_ncols();
    std::vector<std::string> col_names = math_problem->get_col_names(0, n_cols-1);

    ASSERT_EQ(n_cols, 1);
    ASSERT_EQ(col_names.size(), 1);
    ASSERT_EQ(col_names.at(0), "C0000000");
    auto *lower_bounds=new double[n_cols];
    math_problem->get_lb(lower_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(lower_bounds[0], -1000);
    auto *upper_bounds=new double[n_cols];
    math_problem->get_ub(upper_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(upper_bounds[0], 1000);

}

TEST_F(ProblemModifierTest, ResetBounds) {
    ColumnToChange column = {0, 0};
    ColumnsToChange columns_to_change= {column};

    auto problem_modifier = ProblemModifier();
    problem_modifier.setProblem(math_problem);
    problem_modifier.changeProblem(columns_to_change);

    int n_cols = math_problem->get_ncols();
    auto *upper_bounds=new double[n_cols];
    math_problem->get_ub(upper_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(upper_bounds[0], 1e20);
    auto *lower_bounds=new double[n_cols];
    math_problem->get_lb(lower_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(lower_bounds[0], -1e20);
}


TEST_F(ProblemModifierTest, TestName) {
    const ColumnToChange column = {0, 0};
    const ColumnsToChange columns_to_change = {column};
    const Cand candidate = {"candy1"};
    const Cands candidates_link_0 = {candidate};
    const ActiveLink active_link= {0, "tot", candidates_link_0, columns_to_change};

    ActiveLinks active_links;
    active_links.add_link(active_link);



    auto problem_modifier = ProblemModifier();
    problem_modifier.setProblem(math_problem);
    problem_modifier.changeProblem(active_links);

    int n_cols = math_problem->get_ncols();
    auto *upper_bounds=new double[n_cols];
    math_problem->get_ub(upper_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(upper_bounds[0], 1e20);
    auto *lower_bounds=new double[n_cols];
    math_problem->get_lb(lower_bounds, 0, 0);
    ASSERT_DOUBLE_EQ(lower_bounds[0], -1e20);
}


