#include "gtest/gtest.h"
#include <multisolver_interface/SolverFactory.h>
#include "SensitivityPbModifier.h"
#include "BendersBase.h"

const int peak_id = 0;
const int semibase_id = 1;
const int alpha_id = 2;
const int alpha_0_id = 3;
const int alpha_1_id = 4;

const int peak_cost = 60000;
const int semibase_cost = 90000;

std::string peak_name;
std::string semibase_name;
std::string alpha_name;
std::string alpha_0_name;
std::string alpha_1_name;

struct SolverData
{
    SolverAbstract::Ptr solver_model;
    int n_cols;
    int n_rows;
    int n_elems;
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
};

class SensitivityProblemModifierTest : public ::testing::Test
{
public:
    std::vector<SolverData> solvers_data;

protected:
    void SetUp() override
    {
        std::string last_master_mps_path = "../data_test/mps/master_last_iteration.mps";
        std::string solver_name = "CBC";
        SolverFactory factory;

        SolverAbstract::Ptr solver_model = factory.create_solver(solver_name);
        solver_model->init();
        solver_model->read_prob_mps(last_master_mps_path);

        solvers_data.push_back(init_solver_data_from_solver_model(solver_model));
    }

    void TearDown() override {}

    SolverData init_solver_data_from_solver_model(const SolverAbstract::Ptr &solver_model)
    {
        SolverData solver_data;
        solver_data.solver_model = solver_model;
        solver_data.n_cols = -1;
        solver_data.n_rows = -1;
        solver_data.n_elems = -1;
        return solver_data;
    }

    void update_n_cols()
    {
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.n_cols == -1)
            {
                solver_data.n_cols = solver_data.solver_model->get_ncols();
            }
        }
    }
    void update_n_rows()
    {
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.n_rows == -1)
            {
                solver_data.n_rows = solver_data.solver_model->get_nrows();
            }
        }
    }
    void update_n_elems()
    {
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.n_elems == -1)
            {
                solver_data.n_elems = solver_data.solver_model->get_nelems();
            }
        }
    }
    void update_col_names()
    {
        update_n_cols();
        for (auto &solver_data : solvers_data)
        {
            solver_data.col_names = solver_data.solver_model->get_col_names(0, solver_data.n_cols - 1);
        }
    }
    void update_col_type()
    {
        update_n_cols();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.coltypes.size() != solver_data.n_cols)
            {
                std::vector<char> buffer(solver_data.n_cols, '0');
                solver_data.coltypes.insert(coltypes.begin(), buffer.begin(), buffer.end());
                solver_data.solver_model->get_col_type(solver_data.coltypes.data(), 0, solver_data.n_cols - 1);
            }
        }
    }
    void update_row_type()
    {
        update_n_rows();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.rowtypes.size() != solver_data.n_rows)
            {
                std::vector<char> buffer(solver_data.n_rows, '0');
                solver_data.rowtypes.insert(solver_data.rowtypes.begin(), buffer.begin(), buffer.end());
                solver_data.solver_model->get_row_type(solver_data.rowtypes.data(), 0, solver_data.n_rows - 1);
            }
        }
    }
    void update_objectives()
    {
        update_n_cols();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.objectives.size() != solver_data.n_cols)
            {
                std::vector<double> buffer(solver_data.n_cols, -777);
                solver_data.objectives.insert(solver_data.objectives.begin(), buffer.begin(), buffer.end());
                solver_data.solver_model->get_obj(solver_data.objectives.data(), 0, solver_data.n_cols - 1);
            }
        }
    }
    void update_lower_bounds()
    {
        update_n_cols();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.lower_bounds.size() != solver_data.n_cols)
            {
                std::vector<double> buffer(solver_data.n_cols, -777);
                solver_data.lower_bounds.insert(solver_data.lower_bounds.begin(), buffer.begin(), buffer.end());
                solver_data.solver_model->get_lb(solver_data.lower_bounds.data(), 0, solver_data.n_cols - 1);
            }
        }
    }
    void update_upper_bounds()
    {
        update_n_cols();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.upper_bounds.size() != solver_data.n_cols)
            {
                std::vector<double> buffer(solver_data.n_cols, -777);
                solver_data.upper_bounds.insert(solver_data.upper_bounds.begin(), buffer.begin(), buffer.end());
                solver_data.solver_model->get_ub(solver_data.upper_bounds.data(), 0, solver_data.n_cols - 1);
            }
        }
    }
    void update_mat_val()
    {
        update_n_rows();
        update_n_elems();
        for (auto &solver_data : solvers_data)
        {
            if (solver_data.mat_val.size() != solver_data.n_elems)
            {
                std::vector<double> buffer(solver_data.n_elems, -777);
                solver_data.mat_val.insert(solver_data.mat_val.begin(), buffer.begin(), buffer.end());

                solver_data.start_indexes.clear();
                solver_data.start_indexes = std::vector<int>(solver_data.n_rows + 1);
                solver_data.col_indexes.clear();
                solver_data.col_indexes = std::vector<int>(solver_data.n_elems);
                int n_returned(0);
                solver_data.solver_model->get_rows(solver_data.start_indexes.data(), solver_data.col_indexes.data(), solver_data.mat_val.data(),
                                                   solver_data.n_elems, &n_returned, 0, solver_data.n_rows - 1);
            }
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
            solver_model->get_rhs(rhs.data(), 0, n_rows - 1);
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

    if (solver_model->get_solver_name() == "XPRESS")
    {
        peak_name = "peak    ";
        semibase_name = "semibase";
        alpha_name = "alpha   ";
        alpha_0_name = "alpha_0 ";
        alpha_1_name = "alpha_1 ";
    }
    else
    {
        peak_name = "peak";
        semibase_name = "semibase";
        alpha_name = "alpha";
        alpha_0_name = "alpha_0";
        alpha_1_name = "alpha_1";
    }

    double epsilon = 10;
    double best_ub = 1000;
    std::map<int, std::string> id_to_name = {{peak_id, "peak"}, {semibase_id, "semibase"}};

    auto problem_modifier = SensitivityPbModifier(epsilon, best_ub);

    verify_columns_are(5);
    verify_rows_are(5);

    auto sensitivity_pb = problem_modifier.changeProblem(id_to_name, solver_model);

    verify_columns_are(5);
    verify_rows_are(5);

    verify_column(peak_id, peak_name, 'C', peak_cost, 0, 3000);
    verify_column(semibase_id, semibase_name, 'C', semibase_cost, 0, 400);
    verify_column(alpha_id, alpha_name, 'C', 0, -60, 45);
    verify_column(alpha_0_id, alpha_0_name, 'C', 0, -100, 150);
    verify_column(alpha_1_id, alpha_1_name, 'C', 0, -200, 200);

    verify_row(0, 'E', {1, -1, -1}, {alpha_id, alpha_0_id, alpha_1_id}, 0);
    verify_row(1, 'L', {-10, -100, -1}, {peak_id, semibase_id, alpha_0_id}, -1000);
    verify_row(2, 'L', {-20, -100, -1}, {peak_id, semibase_id, alpha_1_id}, -2000);
    verify_row(3, 'L', {-50, -1}, {semibase_id, alpha_0_id}, -3000);
    verify_row(4, 'L', {-20, -1}, {semibase_id, alpha_1_id}, -4000);
    // verify_row(5, 'L', {peak_cost, semibase_cost, 1}, {peak_id, semibase_id, alpha_id}, epsilon + best_ub);
}