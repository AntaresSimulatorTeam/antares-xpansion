#include "gtest/gtest.h"
#include <multisolver_interface/SolverFactory.h>
#include "ProblemModifierCapex.h"
#include "ProblemModifierProjection.h"
#include "BendersBase.h"

const int peak_id = 0;
const int semibase_id = 1;
const int alpha_id = 2;
const int alpha_0_id = 3;
const int alpha_1_id = 4;

const int peak_cost = 10;
const int semibase_cost = 90;

std::string peak_name;
std::string semibase_name;
std::string alpha_name;
std::string alpha_0_name;
std::string alpha_1_name;

std::map<int, std::string> name_to_id;

const double epsilon = 10;
const double best_ub = 1000;

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
    SolverData lastMasterData;

protected:
    void SetUp() override
    {
        std::string data_test_dir = "data_test/sensitivity";
        std::string last_master_mps_path = data_test_dir + "/toy_last_iteration.mps";
        std::string solver_name = "CBC";
        SolverFactory factory;

        auto solver_log_manager = SolverLogManager(std::tmpnam(nullptr));
        SolverAbstract::Ptr solver_model =
            factory.create_solver(solver_name, solver_log_manager);
        solver_model->read_prob_mps(last_master_mps_path);

        lastMasterData = init_solver_data_from_solver_model(solver_model);

        if (solver_name == "XPRESS")
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

        name_to_id = {{peak_id, peak_name}, {semibase_id, semibase_name}};
    }

    void TearDown() override {}

    SolverData init_solver_data_from_solver_model(const SolverAbstract::Ptr &solver_model)
    {
        SolverData solver_data;
        solver_data.solver_model = solver_model;
        solver_data.n_cols = -1;
        solver_data.n_rows = -1;
        solver_data.n_elems = -1;
        solver_data.coltypes = {};
        solver_data.rowtypes = {};
        solver_data.objectives = {};
        solver_data.upper_bounds = {};
        solver_data.lower_bounds = {};
        solver_data.rhs = {};
        solver_data.mat_val = {};
        solver_data.col_indexes = {};
        solver_data.start_indexes = {};
        solver_data.col_names = {};
        return solver_data;
    }

    void update_n_cols(SolverData &solver_data)
    {
        if (solver_data.n_cols == -1)
        {
            solver_data.n_cols = solver_data.solver_model->get_ncols();
        }
    }
    void update_n_rows(SolverData &solver_data)
    {
        if (solver_data.n_rows == -1)
        {
            solver_data.n_rows = solver_data.solver_model->get_nrows();
        }
    }
    void update_n_elems(SolverData &solver_data)
    {
        if (solver_data.n_elems == -1)
        {
            solver_data.n_elems = solver_data.solver_model->get_nelems();
        }
    }
    void update_col_names(SolverData &solver_data)
    {
        update_n_cols(solver_data);
        solver_data.col_names = solver_data.solver_model->get_col_names(0, solver_data.n_cols - 1);
    }
    void update_col_type(SolverData &solver_data)
    {
        update_n_cols(solver_data);
        if (solver_data.coltypes.size() != solver_data.n_cols)
        {
            std::vector<char> buffer(solver_data.n_cols, '0');
            solver_data.coltypes.insert(solver_data.coltypes.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_col_type(solver_data.coltypes.data(), 0, solver_data.n_cols - 1);
        }
    }
    void update_row_type(SolverData &solver_data)
    {
        update_n_rows(solver_data);
        if (solver_data.rowtypes.size() != solver_data.n_rows)
        {
            std::vector<char> buffer(solver_data.n_rows, '0');
            solver_data.rowtypes.insert(solver_data.rowtypes.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_row_type(solver_data.rowtypes.data(), 0, solver_data.n_rows - 1);
        }
    }
    void update_objectives(SolverData &solver_data)
    {
        update_n_cols(solver_data);
        if (solver_data.objectives.size() != solver_data.n_cols)
        {
            std::vector<double> buffer(solver_data.n_cols, -777);
            solver_data.objectives.insert(solver_data.objectives.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_obj(solver_data.objectives.data(), 0, solver_data.n_cols - 1);
        }
    }
    void update_lower_bounds(SolverData &solver_data)
    {
        update_n_cols(solver_data);
        if (solver_data.lower_bounds.size() != solver_data.n_cols)
        {
            std::vector<double> buffer(solver_data.n_cols, -777);
            solver_data.lower_bounds.insert(solver_data.lower_bounds.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_lb(solver_data.lower_bounds.data(), 0, solver_data.n_cols - 1);
        }
    }
    void update_upper_bounds(SolverData &solver_data)
    {
        update_n_cols(solver_data);
        if (solver_data.upper_bounds.size() != solver_data.n_cols)
        {
            std::vector<double> buffer(solver_data.n_cols, -777);
            solver_data.upper_bounds.insert(solver_data.upper_bounds.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_ub(solver_data.upper_bounds.data(), 0, solver_data.n_cols - 1);
        }
    }
    void update_mat_val(SolverData &solver_data)
    {
        update_n_rows(solver_data);
        update_n_elems(solver_data);
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

    std::vector<double> getRowCoefficients(SolverData &solver_data, int index_row)
    {
        update_mat_val(solver_data);
        std::vector<double> row;
        row.insert(row.begin(), solver_data.mat_val.begin() + solver_data.start_indexes.at(index_row), solver_data.mat_val.begin() + solver_data.start_indexes.at(index_row + 1));
        return row;
    }

    std::vector<int> getRowColIndexes(SolverData &solver_data, int index_row)
    {
        update_mat_val(solver_data);
        std::vector<int> index;
        index.insert(index.begin(), solver_data.col_indexes.begin() + solver_data.start_indexes.at(index_row), solver_data.col_indexes.begin() + solver_data.start_indexes.at(index_row + 1));
        return index;
    }

    void update_rhs_val(SolverData &solver_data)
    {
        update_n_rows(solver_data);
        if (solver_data.rhs.size() != solver_data.n_rows)
        {
            std::vector<double> buffer(solver_data.n_rows, -777);
            solver_data.rhs.insert(solver_data.rhs.begin(), buffer.begin(), buffer.end());
            solver_data.solver_model->get_rhs(solver_data.rhs.data(), 0, solver_data.n_rows - 1);
        }
    }
    void verify_columns_are(SolverData &solver_data, const int expected_n_cols)
    {
        update_n_cols(solver_data);
        ASSERT_EQ(solver_data.n_cols, expected_n_cols);
    }
    void verify_rows_are(SolverData &solver_data, const int expected_n_rows)
    {
        update_n_rows(solver_data);
        ASSERT_EQ(solver_data.n_rows, expected_n_rows);
    }
    void verify_column_name_is(SolverData &solver_data, const int col_id, std::basic_string<char> name)
    {
        update_col_names(solver_data);
        EXPECT_EQ(solver_data.col_names.at(col_id), name);
    }
    void verify_column_is_of_type(SolverData &solver_data, const int col_id, char type)
    {
        update_col_type(solver_data);
        EXPECT_EQ(solver_data.coltypes.at(col_id), type);
    }
    void verify_column_objective_is(SolverData &solver_data, const int col_id, double obj_value)
    {
        update_objectives(solver_data);
        EXPECT_DOUBLE_EQ(solver_data.objectives.at(col_id), obj_value);
    }
    void verify_column_lower_bound_is(SolverData &solver_data, const int col_id, double lower_value)
    {
        update_lower_bounds(solver_data);
        EXPECT_DOUBLE_EQ(solver_data.lower_bounds.at(col_id), lower_value);
    }
    void verify_column_upper_bound_is(SolverData &solver_data, const int col_id, double upper_value)
    {
        update_upper_bounds(solver_data);
        EXPECT_DOUBLE_EQ(solver_data.upper_bounds.at(col_id), upper_value);
    }
    void verify_row_is_of_type(SolverData &solver_data, const int row_id, char type)
    {
        update_row_type(solver_data);
        EXPECT_EQ(solver_data.rowtypes.at(row_id), type);
    }
    void verify_rhs_is(SolverData &solver_data, const int rhs_id, double rhs_value)
    {
        update_rhs_val(solver_data);
        EXPECT_DOUBLE_EQ(solver_data.rhs.at(rhs_id), rhs_value);
    }
    void verify_column(SolverData &solver_data, const int col_id, std::basic_string<char> name, char type, double obj_value, double lower_value, double upper_value)
    {
        verify_column_name_is(solver_data, col_id, name);
        verify_column_is_of_type(solver_data, col_id, type);
        verify_column_objective_is(solver_data, col_id, obj_value);
        verify_column_lower_bound_is(solver_data, col_id, lower_value);
        verify_column_upper_bound_is(solver_data, col_id, upper_value);
    }
    void verify_row(SolverData &solver_data, int row, char type, const std::vector<double> &coeff, const std::vector<int> &col_indexes, double rhs)
    {
        verify_row_is_of_type(solver_data, row, type);
        EXPECT_EQ(getRowCoefficients(solver_data, row), coeff);
        EXPECT_EQ(getRowColIndexes(solver_data, row), col_indexes);
        verify_rhs_is(solver_data, row, rhs);
    }
    void verify_column_number_equality(SolverData &lastMasterData, SolverData &sensitivityPbData)
    {
        update_n_cols(lastMasterData);
        update_n_cols(sensitivityPbData);
        ASSERT_EQ(lastMasterData.n_cols, sensitivityPbData.n_cols);
    }
    void verify_column_name_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        update_col_names(lastMasterData);
        update_col_names(sensitivityPbData);
        EXPECT_EQ(lastMasterData.col_names.at(col_id), sensitivityPbData.col_names.at(col_id));
    }
    void verify_column_type_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        update_col_type(lastMasterData);
        update_col_type(sensitivityPbData);
        EXPECT_EQ(lastMasterData.coltypes.at(col_id), sensitivityPbData.coltypes.at(col_id));
    }
    void verify_column_objective_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        update_objectives(lastMasterData);
        update_objectives(sensitivityPbData);
        EXPECT_DOUBLE_EQ(lastMasterData.objectives.at(col_id), sensitivityPbData.objectives.at(col_id));
    }
    void verify_column_lower_bound_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        update_lower_bounds(lastMasterData);
        update_lower_bounds(sensitivityPbData);
        EXPECT_DOUBLE_EQ(lastMasterData.lower_bounds.at(col_id), sensitivityPbData.lower_bounds.at(col_id));
    }
    void verify_column_upper_bound_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        update_upper_bounds(lastMasterData);
        update_upper_bounds(sensitivityPbData);
        EXPECT_DOUBLE_EQ(lastMasterData.upper_bounds.at(col_id), sensitivityPbData.upper_bounds.at(col_id));
    }
    void verify_column_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        verify_column_name_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_type_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_objective_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_lower_bound_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_upper_bound_equality(lastMasterData, sensitivityPbData, col_id);
    }
    void verify_column_objective_update(SolverData &lastMasterData, SolverData &sensitivityPbData, const int col_id)
    {
        verify_column_name_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_type_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_lower_bound_equality(lastMasterData, sensitivityPbData, col_id);
        verify_column_upper_bound_equality(lastMasterData, sensitivityPbData, col_id);

        update_objectives(sensitivityPbData);
        EXPECT_DOUBLE_EQ(sensitivityPbData.objectives.at(col_id), 0);
    }
    void verify_row_is_added(SolverData &lastMasterData, SolverData &sensitivityPbData)
    {
        update_n_rows(lastMasterData);
        update_n_rows(sensitivityPbData);
        ASSERT_EQ(lastMasterData.n_rows + 1, sensitivityPbData.n_rows);
    }
    void verify_row_type_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int row_id)
    {
        update_row_type(lastMasterData);
        update_row_type(sensitivityPbData);
        EXPECT_EQ(lastMasterData.rowtypes.at(row_id), sensitivityPbData.rowtypes.at(row_id));
    }
    void verify_rhs_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int row_id)
    {
        update_rhs_val(lastMasterData);
        update_rhs_val(sensitivityPbData);
        EXPECT_DOUBLE_EQ(lastMasterData.rhs.at(row_id), sensitivityPbData.rhs.at(row_id));
    }
    void verify_row_equality(SolverData &lastMasterData, SolverData &sensitivityPbData, const int row_id)
    {
        verify_row_type_equality(lastMasterData, sensitivityPbData, row_id);
        EXPECT_EQ(getRowCoefficients(lastMasterData, row_id), getRowCoefficients(sensitivityPbData, row_id));
        EXPECT_EQ(getRowColIndexes(lastMasterData, row_id), getRowColIndexes(sensitivityPbData, row_id));
        verify_rhs_equality(lastMasterData, sensitivityPbData, row_id);
    }
    void verify_additional_row(SolverData &sensitivityPbData)
    {
        update_n_rows(sensitivityPbData);
        verify_row(sensitivityPbData, sensitivityPbData.n_rows - 1, 'L', {peak_cost, semibase_cost, 1, 0, 0}, {peak_id, semibase_id, alpha_id, alpha_0_id, alpha_1_id}, epsilon + best_ub);
    }
    void verify_last_master_problem(SolverData &solver_data)
    {
        verify_columns_are(solver_data, 5);
        verify_rows_are(solver_data, 5);

        verify_column(solver_data, peak_id, peak_name, 'C', peak_cost, 0, 3000);
        verify_column(solver_data, semibase_id, semibase_name, 'C', semibase_cost, 0, 400);
        verify_column(solver_data, alpha_id, alpha_name, 'C', 1, -60, 1000);
        verify_column(solver_data, alpha_0_id, alpha_0_name, 'C', 0, -100, 150);
        verify_column(solver_data, alpha_1_id, alpha_1_name, 'C', 0, -200, 2000);

        verify_row(solver_data, 0, 'E', {1, -1, -1}, {alpha_id, alpha_0_id, alpha_1_id}, 0);
        verify_row(solver_data, 1, 'L', {-10, -1, -1}, {peak_id, semibase_id, alpha_0_id}, -300);
        verify_row(solver_data, 2, 'L', {-25, -2, -1}, {peak_id, semibase_id, alpha_1_id}, -400);
        verify_row(solver_data, 3, 'L', {-20, -1}, {semibase_id, alpha_0_id}, -350);
        verify_row(solver_data, 4, 'L', {-30, -1}, {semibase_id, alpha_1_id}, -500);
    }
    void verify_sensitivity_problem(SolverData &lastMasterData, SolverData &sensitivityPbData)
    {
        verify_column_number_equality(lastMasterData, sensitivityPbData);
        for (int col_id(0); col_id < lastMasterData.n_cols; col_id++)
        {
            if (col_id != alpha_id)
            {
                verify_column_equality(lastMasterData, sensitivityPbData, col_id);
            }
        }
        verify_column_objective_update(lastMasterData, sensitivityPbData, alpha_id);

        verify_row_is_added(lastMasterData, sensitivityPbData);
        for (int row_id(0); row_id < lastMasterData.n_rows; row_id++)
        {
            verify_row_equality(lastMasterData, sensitivityPbData, row_id);
        }
        verify_additional_row(sensitivityPbData);
    }
};

TEST_F(SensitivityProblemModifierTest, ChangeProblemCapex)
{
    int nb_candidates = name_to_id.size();
    verify_last_master_problem(lastMasterData);

    auto problem_modifier = std::make_shared<ProblemModifierCapex>(epsilon, best_ub, lastMasterData.solver_model);
    auto sensitivity_pb = problem_modifier->changeProblem(nb_candidates);

    SolverData sensitivityPbData = init_solver_data_from_solver_model(sensitivity_pb);

    verify_sensitivity_problem(lastMasterData, sensitivityPbData);
}