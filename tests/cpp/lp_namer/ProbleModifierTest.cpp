
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
    int n_cols=-1;
    int n_rows=-1;
    std::vector<char> coltypes;
    std::vector<double> objectives;
    std::vector<double> upper_bounds;
    std::vector<double> lower_bounds;
    std::vector<std::basic_string<char>> col_names;


protected:
    void SetUp()
    {

        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        std::vector<double> objectives(1, 1);
        std::vector<double> lb(1, -1000);
        std::vector<double> ub(1, 1000);
        std::vector<char> coltypes(1, 'C');
        std::vector<int> mstart(1, 0);
        std::vector<std::string> candidates_colnames(1, "C0000000");
        solver_addcols(math_problem, objectives, mstart, {}, {}, lb, ub, coltypes, candidates_colnames);
    }

    void TearDown()
    {
        // tear down
    }

    void update_n_cols(){
        if(n_rows==-1){
            n_cols = math_problem->get_ncols();
        }
    }
    void update_n_rows(){
        if(n_rows==-1){
            n_rows = math_problem->get_nrows();
        }
    }
    void update_col_names(){
        update_n_cols();
        col_names = math_problem->get_col_names(0, n_cols-1);
    }
    void update_col_type(){
        update_n_cols();
        if(coltypes.size()!= n_cols){
            std::vector<char> buffer(n_cols, '0');
            coltypes.insert(coltypes.begin(), buffer.begin(), buffer.end());
            math_problem->get_col_type(coltypes.data(), 0, n_cols-1);
        }
    }
    void update_objectives(){
        update_n_cols();
        if(objectives.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            objectives.insert(objectives.begin(), buffer.begin(), buffer.end());
            math_problem->get_obj(objectives.data(), 0, n_cols-1);
        }
    }
    void update_lower_bounds(){
        update_n_cols();
        if(lower_bounds.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            lower_bounds.insert(lower_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_lb(lower_bounds.data(), 0, n_cols-1);
        }
    }
    void update_upper_bounds(){
        update_n_cols();
        if(upper_bounds.size()!=n_cols){
            std::vector<double> buffer(n_cols, -777);
            upper_bounds.insert(upper_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_ub(upper_bounds.data(), 0, n_cols-1);
        }
    }
    void verify_columns_are(const int expected_n_cols){
        update_n_cols();
        ASSERT_EQ(n_cols, expected_n_cols);
    }
    void verify_rows_are(const int expected_n_rows){
        update_n_rows();
        ASSERT_EQ(n_rows, expected_n_rows);
    }
    void verify_column_name_is(const int col_id, std::basic_string<char> name){
        update_col_names();
        ASSERT_EQ(col_names.at(col_id), name);
    }
    void verify_column_is_of_type(const int col_id, char type){
        update_col_type();
        ASSERT_EQ(coltypes.at(col_id), type);
    }
    void verify_column_objective_is(const int col_id, double obj_value){
        update_objectives();
        ASSERT_DOUBLE_EQ(objectives.at(col_id), obj_value);
    }
    void verify_column_lower_bound_is(const int col_id, double lower_value){
        update_lower_bounds();
        ASSERT_DOUBLE_EQ(lower_bounds.at(col_id), lower_value);
    }
    void verify_column_upper_bound_is(const int col_id, double upper_value){
        update_upper_bounds();
        ASSERT_DOUBLE_EQ(upper_bounds.at(col_id), upper_value);
    }
};

TEST_F(ProblemModifierTest, empty_test_the_multisolver_interface)
{
    verify_columns_are(1);
    verify_rows_are(0);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1000);
    verify_column_upper_bound_is(0, 1000);

    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    rstart.push_back(dmatval.size());

}


TEST_F(ProblemModifierTest, One_link_no_candidates_link_boundaries_are_removed) {
    const ColumnToChange column = {0, 0};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{0,{column}}};
    const Cands candidates_link_0 = {};
    const ActiveLink_AS active_link= {0, candidates_link_0};

    ActiveLinks_AS active_links;
    active_links.add_link(active_link);

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), active_links, p_var_columns);

    verify_columns_are(1);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1e20);
    verify_column_upper_bound_is(0, 1e20);
}


TEST_F(ProblemModifierTest, One_link_two_candidates) {
    const ColumnToChange column = {0, 0};
    const std::map<linkId , ColumnsToChange> p_var_columns = {{0,{column}}};
    const Cands candidates_link_0 = {{"candy1"}, {"candy2"}};
    const ActiveLink_AS active_link_0= {0, candidates_link_0};

    ActiveLinks_AS active_links;
    active_links.add_link(active_link_0);

    auto problem_modifier = ProblemModifier();
    math_problem = problem_modifier.changeProblem(std::move(math_problem), active_links, p_var_columns);

    verify_columns_are(3);

    verify_column_name_is(0, "C0000000");
    verify_column_is_of_type(0, 'C');
    verify_column_objective_is(0, 1);
    verify_column_lower_bound_is(0, -1e20);
    verify_column_upper_bound_is(0, 1e20);


    verify_column_name_is(1, "candy1");
    verify_column_is_of_type(1, 'C');
    verify_column_objective_is(1, 0);
    verify_column_lower_bound_is(1, -1e20);
    verify_column_upper_bound_is(1, 1e20);

    verify_column_name_is(2, "candy2");
    verify_column_is_of_type(2, 'C');
    verify_column_objective_is(2, 0);
    verify_column_lower_bound_is(2, -1e20);
    verify_column_upper_bound_is(2, 1e20);


}


