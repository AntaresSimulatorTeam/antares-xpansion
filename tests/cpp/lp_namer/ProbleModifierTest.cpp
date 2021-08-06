
#include <ProblemModifier.h>
#include <multisolver_interface/SolverFactory.h>
#include <fstream>
#include <solver_utils.h>
#include "gtest/gtest.h"


class ProblemModifierTest : public ::testing::Test
{
public:
    const std::string mps_name = "dummy.mps";
    SolverAbstract::Ptr math_problem;

protected:
    void SetUp()
    {

        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        bool usefile = false;
        if(usefile){
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
            math_problem->read_prob_mps(mps_name);
            std::remove(mps_name.c_str());
        } else{
            std::vector<double> objectives(1, 1);
            std::vector<double> lb(1, -1000);
            std::vector<double> ub(1, 1000);
            std::vector<char> coltypes(1, 'C');
            std::vector<int> mstart(1, 0);
            std::vector<std::string> candidates_colnames(1, "C0000000");
            solver_addcols(math_problem, objectives, mstart, {}, {}, lb, ub, coltypes, candidates_colnames);
        }
    }

    void TearDown()
    {
        // tear down
    }
};

TEST_F(ProblemModifierTest, empty_test_the_multisolver_interface)
{
    int n_cols = math_problem->get_ncols();
    ASSERT_EQ(n_cols, 1);

    std::vector<char> coltypes(n_cols, '0');
    std::vector<double> objectives(n_cols, 777);
    std::vector<double> upper_bounds(n_cols, 777);
    std::vector<double> lower_bounds(n_cols, 777);

    auto col_names = math_problem->get_col_names(0, n_cols-1);
    math_problem->get_col_type(coltypes.data(), 0, n_cols-1);
    math_problem->get_obj(objectives.data(), 0, n_cols-1);
    math_problem->get_ub(upper_bounds.data(), 0, n_cols-1);
    math_problem->get_lb(lower_bounds.data(), 0, n_cols-1);

    ASSERT_EQ(n_cols, 1);
    ASSERT_EQ(col_names.size(), 1);
    ASSERT_EQ(col_names.at(0), "C0000000");
    ASSERT_EQ(coltypes.at(0), 'C');
    ASSERT_DOUBLE_EQ(objectives.at(0), 1);
    ASSERT_DOUBLE_EQ(lower_bounds.at(0), -1000);
    ASSERT_DOUBLE_EQ(upper_bounds.at(0), 1000);
}


TEST_F(ProblemModifierTest, One_link_no_candidates_link_boundaries_are_removed) {
    const ColumnToChange column = {0, 0};
    const ColumnsToChange columns_to_change = {column};
    const Cands candidates_link_0 = {};
    const ActiveLink active_link= {0, candidates_link_0, columns_to_change};

    ActiveLinks active_links;
    active_links.add_link(active_link);

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), active_links);

    int n_cols = math_problem->get_ncols();
    std::vector<double> upper_bounds(n_cols, 777);
    std::vector<double> lower_bounds(n_cols, 777);
    math_problem->get_ub(upper_bounds.data(), 0, n_cols-1);
    math_problem->get_lb(lower_bounds.data(), 0, n_cols-1);


    ASSERT_EQ(n_cols, 1);
    ASSERT_DOUBLE_EQ(upper_bounds.at(0), 1e20);
    ASSERT_DOUBLE_EQ(lower_bounds.at(0), -1e+20);
}


TEST_F(ProblemModifierTest, One_link_two_candidates) {
    const ColumnToChange column = {0, 0};
    const ColumnsToChange columns_to_change = {column};
    const Cands candidates_link_0 = {{"candy1"}, {"candy2"}};
    const ActiveLink active_link_0= {0, candidates_link_0, columns_to_change};

    ActiveLinks active_links;
    active_links.add_link(active_link_0);

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), active_links);

    int n_cols = math_problem->get_ncols();
    ASSERT_EQ(n_cols, 3);

    std::vector<char> coltypes(n_cols, '0');
    std::vector<double> objectives(n_cols, 777);
    std::vector<double> upper_bounds(n_cols, 777);
    std::vector<double> lower_bounds(n_cols, 777);

    auto col_names = math_problem->get_col_names(0, n_cols-1);
    math_problem->get_col_type(coltypes.data(), 0, n_cols-1);
    math_problem->get_obj(objectives.data(), 0, n_cols-1);
    math_problem->get_ub(upper_bounds.data(), 0, n_cols-1);
    math_problem->get_lb(lower_bounds.data(), 0, n_cols-1);

    ASSERT_DOUBLE_EQ(upper_bounds.at(0), 1e20);
    ASSERT_DOUBLE_EQ(lower_bounds.at(0), -1e+20);

    ASSERT_EQ(col_names.at(1), "candy1");
    ASSERT_EQ(coltypes.at(1), 'C');
    ASSERT_DOUBLE_EQ(objectives.at(1), 0);
    ASSERT_DOUBLE_EQ(upper_bounds.at(1), 1e20);
    ASSERT_DOUBLE_EQ(lower_bounds.at(1), -1e+20);

    ASSERT_EQ(col_names.at(2), "candy2");
    ASSERT_EQ(coltypes.at(2), 'C');
    ASSERT_DOUBLE_EQ(objectives.at(2), 0);
    ASSERT_DOUBLE_EQ(upper_bounds.at(2), 1e20);
    ASSERT_DOUBLE_EQ(lower_bounds.at(2), -1e+20);
}


