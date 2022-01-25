#include "gtest/gtest.h"
#include <multisolver_interface/SolverFactory.h>
#include "SensitivityPbModifier.h"
#include "BendersBase.h"

class SensitivityProblemModifierTest : public ::testing::Test
{
public:
    SolverAbstract::Ptr math_problem;
    int n_cols = -1;
    int n_rows = -1;
    int n_elems = -1;
    std::vector<char> coltypes;
    std::vector<char> rowtypes;
    std::vector<double> objectives;
    std::vector<double> upper_bounds;
    std::vector<double> lower_bounds;
    std::vector<double> rhs;
    std::vector<double> mat_val;
    std::vector<int> col_indexes;
    std::vector<int> start_indexes;
    std::vector<std::basic_string<char>> col_names;

protected:
    void SetUp()
    {
        std::string last_master_mps_path = "../data_test/mps/master_last_iteration.mps";
        std::string solver_name = "XPRESS";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        math_problem->init();
        math_problem->read_prob_mps(last_master_mps_path);
    }

    void TearDown()
    {
        n_cols = -1;
        n_rows = -1;
        n_elems = -1;
    }

    void update_n_cols()
    {
        if (n_cols == -1)
        {
            n_cols = math_problem->get_ncols();
        }
    }
    void update_n_rows()
    {
        if (n_rows == -1)
        {
            n_rows = math_problem->get_nrows();
        }
    }
    void update_n_elems()
    {
        if (n_elems == -1)
        {
            n_elems = math_problem->get_nelems();
        }
    }
    void update_col_names()
    {
        update_n_cols();
        col_names = math_problem->get_col_names(0, n_cols - 1);
    }
    void update_col_type()
    {
        update_n_cols();
        if (coltypes.size() != n_cols)
        {
            std::vector<char> buffer(n_cols, '0');
            coltypes.insert(coltypes.begin(), buffer.begin(), buffer.end());
            math_problem->get_col_type(coltypes.data(), 0, n_cols - 1);
        }
    }
    void update_row_type()
    {
        update_n_rows();
        if (rowtypes.size() != n_rows)
        {
            std::vector<char> buffer(n_rows, '0');
            rowtypes.insert(rowtypes.begin(), buffer.begin(), buffer.end());
            math_problem->get_row_type(rowtypes.data(), 0, n_rows - 1);
        }
    }
    void update_objectives()
    {
        update_n_cols();
        if (objectives.size() != n_cols)
        {
            std::vector<double> buffer(n_cols, -777);
            objectives.insert(objectives.begin(), buffer.begin(), buffer.end());
            math_problem->get_obj(objectives.data(), 0, n_cols - 1);
        }
    }
    void update_lower_bounds()
    {
        update_n_cols();
        if (lower_bounds.size() != n_cols)
        {
            std::vector<double> buffer(n_cols, -777);
            lower_bounds.insert(lower_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_lb(lower_bounds.data(), 0, n_cols - 1);
        }
    }
    void update_upper_bounds()
    {
        update_n_cols();
        if (upper_bounds.size() != n_cols)
        {
            std::vector<double> buffer(n_cols, -777);
            upper_bounds.insert(upper_bounds.begin(), buffer.begin(), buffer.end());
            math_problem->get_ub(upper_bounds.data(), 0, n_cols - 1);
        }
    }
    void update_mat_val()
    {
        update_n_rows();
        update_n_elems();

        if (mat_val.size() != n_elems)
        {
            std::vector<double> buffer(n_elems, -777);
            mat_val.insert(mat_val.begin(), buffer.begin(), buffer.end());

            start_indexes.clear();
            start_indexes = std::vector<int>(n_rows + 1);
            col_indexes.clear();
            col_indexes = std::vector<int>(n_elems);
            int n_returned(0);
            math_problem->get_rows(start_indexes.data(), col_indexes.data(), mat_val.data(),
                                   n_elems, &n_returned, 0, n_rows - 1);
        }
    }

    std::vector<double> getRowCoefficients(int index_row)
    {
        update_mat_val();
        std::vector<double> row;
        row.insert(row.begin(), mat_val.begin() + start_indexes.at(index_row), mat_val.begin() + start_indexes.at(index_row + 1));
        return row;
    }

    std::vector<int> getRowColIndexes(int index_row)
    {
        update_mat_val();
        std::vector<int> index;
        index.insert(index.begin(), col_indexes.begin() + start_indexes.at(index_row), col_indexes.begin() + start_indexes.at(index_row + 1));
        return index;
    }

    void update_rhs_val()
    {
        update_n_rows();
        if (rhs.size() != n_rows)
        {
            std::vector<double> buffer(n_rows, -777);
            rhs.insert(rhs.begin(), buffer.begin(), buffer.end());
            math_problem->get_rhs(rhs.data(), 0, n_rows - 1);
        }
    }
    void verify_columns_are(const int expected_n_cols)
    {
        update_n_cols();
        ASSERT_EQ(n_cols, expected_n_cols);
    }
    void verify_rows_are(const int expected_n_rows)
    {
        update_n_rows();
        ASSERT_EQ(n_rows, expected_n_rows);
    }
    void verify_column_name_is(const int col_id, std::basic_string<char> name)
    {
        update_col_names();
        ASSERT_EQ(col_names.at(col_id), name);
    }
    void verify_column_is_of_type(const int col_id, char type)
    {
        update_col_type();
        ASSERT_EQ(coltypes.at(col_id), type);
    }
    void verify_column_objective_is(const int col_id, double obj_value)
    {
        update_objectives();
        ASSERT_DOUBLE_EQ(objectives.at(col_id), obj_value);
    }
    void verify_column_lower_bound_is(const int col_id, double lower_value)
    {
        update_lower_bounds();
        ASSERT_DOUBLE_EQ(lower_bounds.at(col_id), lower_value);
    }
    void verify_column_upper_bound_is(const int col_id, double upper_value)
    {
        update_upper_bounds();
        ASSERT_DOUBLE_EQ(upper_bounds.at(col_id), upper_value);
    }
    void verify_row_is_of_type(const int row_id, char type)
    {
        update_row_type();
        ASSERT_EQ(rowtypes.at(row_id), type);
    }
    void verify_rhs_is(const int rhs_id, double rhs_value)
    {
        update_rhs_val();
        ASSERT_DOUBLE_EQ(rhs.at(rhs_id), rhs_value);
    }
    void verify_column(const int col_id, std::basic_string<char> name, char type, double obj_value, double lower_value, double upper_value)
    {
        verify_column_name_is(col_id, name);
        verify_column_is_of_type(col_id, type);
        verify_column_objective_is(col_id, obj_value);
        verify_column_lower_bound_is(col_id, lower_value);
        verify_column_upper_bound_is(col_id, upper_value);
    }
    void verify_row(int row, char type, const std::vector<double> &coeff, const std::vector<int> &col_indexes, double rhs)
    {
        verify_row_is_of_type(row, type);
        ASSERT_EQ(getRowCoefficients(row), coeff);
        ASSERT_EQ(getRowColIndexes(row), col_indexes);
        verify_rhs_is(row, rhs);
    }
};

TEST_F(SensitivityProblemModifierTest, ChangeProblem)
{
    const int peak_id = 0;
    const int semibase_id = 1;
    const int alpha_id = 2;
    const int alpha_0_id = 3;
    const int alpha_1_id = 4;

    std::string peak_name;
    std::string semibase_name;
    std::string alpha_name;
    std::string alpha_0_name;
    std::string alpha_1_name;

    if (math_problem->get_solver_name() == "XPRESS"){
        peak_name = "peak    ";
        semibase_name = "semibase";
        alpha_name = "alpha   ";
        alpha_0_name = "alpha_0 ";
        alpha_1_name = "alpha_1 ";
    }
    else {
        peak_name = "peak";
        semibase_name = "semibase";
        alpha_name = "alpha";
        alpha_0_name = "alpha_0";
        alpha_1_name = "alpha_1";
    }

    double epsilon = 1e4;
    double best_ub = 171696313.74728549;
    std::map<int, std::string> id_to_name = {{peak_id, "peak"}, {semibase_id, "semibase"}};

    auto problem_modifier = SensitivityPbModifier(epsilon, best_ub, id_to_name, math_problem);
    problem_modifier.changeProblem();

    verify_columns_are(5);

    verify_column(peak_id, peak_name, 'C', 60000, 0, 3000);
    verify_column(semibase_id, semibase_name, 'C', 90000, 0, 400);
    verify_column(alpha_id, alpha_name, 'C', 0, -10000000000, 172124607.0861876);
    verify_column(alpha_0_id, alpha_0_name, 'C', 0, -10000000000, 1e+20);
    verify_column(alpha_1_id, alpha_1_name, 'C', 0, -10000000000, 1e+20);

    verify_rows_are(14);

    verify_row(0, 'E', {1, -1, -1}, {alpha_id, alpha_0_id, alpha_1_id}, 0);
    verify_row(1, 'L', {-245032.5320257737, -245032.5263440894, -1}, {peak_id, semibase_id, alpha_0_id}, -117533866.2940578);
    verify_row(2, 'L', {-604145.0150967597, -604144.9688289623, -1}, {peak_id, semibase_id, alpha_1_id}, -457438478.9421513);
    verify_row(3, 'L', {-2174.971812644999, -1}, {semibase_id, alpha_0_id}, -15919303.47465738);
    verify_row(4, 'L', {-4039.965523606698, -1}, {semibase_id, alpha_1_id}, -57334535.85367353);
    verify_row(5, 'L', {-79279.9953740404, -86479.52821613279, -1}, {peak_id, semibase_id, alpha_0_id}, -82383098.27316803);
    verify_row(6, 'L', {-257659.9852173438, -267209.9510115215, -1}, {peak_id, semibase_id, alpha_1_id}, -350311384.9928737);
    verify_row(7, 'L', {-7199.532842092396, -1}, {semibase_id, alpha_0_id}, -16164482.13700069);
    verify_row(8, 'L', {-109009.9937457993, -118559.959539977, -1}, {peak_id, semibase_id, alpha_1_id}, -218399382.5610252);
    verify_row(9, 'L', {-7199.532842092396, -1}, {semibase_id, alpha_0_id}, -16164482.13700069);
    verify_row(10, 'L', {-69369.9960200541, -78919.96181423182, -1}, {peak_id, semibase_id, alpha_1_id}, -169919665.3424386);
    verify_row(11, 'L', {-7199.532842092396, -1}, {semibase_id, alpha_0_id}, -16164482.13700069);
    verify_row(12, 'L', {-39639.9977257452, -49189.96351992292, -1}, {peak_id, semibase_id, alpha_1_id}, -122916538.0391362);
    verify_row(13, 'L', {60000, 90000, 1}, {peak_id, semibase_id, alpha_id}, 171706313.74728549);
}