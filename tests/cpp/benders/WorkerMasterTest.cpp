#include <cmath>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "EmptyLogManager.h"
#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include "antares-xpansion/benders/logger/Master.h"

class NOOPSolverForWorkerMaster: public NOOPSolverForWorker
{
public:
    void setSolverBehavior(const std::vector<double>& sol,
                           const std::vector<char>& types,
                           const std::vector<double>& lower_bounds,
                           const std::vector<double>& upper_bounds)
    {
        solution = sol;
        col_types = types;
        lbs = lower_bounds;
        ubs = upper_bounds;
    }

    SolverAbstract* clone() const override
    {
        return nullptr;
    }

    int get_ncols() const override
    {
        return solution.size();
    }

    void get_lp_sol(double* sol, double*, double*) const override
    {
        std::copy(solution.begin(), solution.end(), sol);
    }

void add_rows(int newrows,
              int newnz,
              const char* qrtype,
              const double* rhs,
              const double* range,
              const int* mstart,
              const int* mclind,
              const double* dmatval,
              const std::vector<std::string>& row_names) override
    {
        last_rowrhs.assign(rhs, rhs + newrows);
        last_matval.assign(dmatval, dmatval + newnz);
    }

    std::vector<double> last_rowrhs;
    std::vector<double> last_matval;

private:
    std::vector<double> solution;
};

class NOOPBendersProblemProvider: public IBendersProblemProvider
{
public:
    NOOPBendersProblemProvider() = default;

    void provide_problem(const SolverIO& solver_io,
                         std::shared_ptr<SolverAbstract> solver) const override
    {
    }

    std::filesystem::path provide_file_path() const override
    {
        return "";
    }
};

class WorkerMasterTest: public ::testing::Test

{
public:
    WorkerMasterTest() = default;

protected:
    Point x_out;
    double overall_cost{0.0};
    DblVector single_costs{0.0};
    DblVector non_subpb_vars{};

    std::shared_ptr<WorkerMaster> init_worker_master(double master_solution_tolerance,
                                                     double cut_coefficient_tolerance) const
    {
        auto test_solver = std::make_shared<NOOPSolverForWorkerMaster>();
        EmptyLogManager solver_log_manager;
        auto problem_provider = std::make_shared<NOOPBendersProblemProvider>();
        std::map<int, double> subproblem_cut_coefficient_tolerance{};
        subproblem_cut_coefficient_tolerance[0] = cut_coefficient_tolerance;
        auto master = std::make_shared<WorkerMaster>(VariableMap{},
                                                     "COIN",
                                                     0,
                                                     1,
                                                     solver_log_manager,
                                                     false,
                                                     std::make_shared<xpansion::logger::Master>(),
                                                     ProblemsFormat::MPS_FILE,
                                                     problem_provider.get(),
                                                     master_solution_tolerance,
                                                     subproblem_cut_coefficient_tolerance);
        master->_solver = test_solver;
        master->_id_to_name = {{0, "var1"}, {1, "var2"}, {2, "var3"}};
        master->_name_to_id = {{"var1", 0}, {"var2", 1}, {"var3", 2}};
        master->set_id_alpha(3);
        master->set_id_single_subpb_costs_under_approx({4});
        master->_id_master_only_vars = {};
        return master;
    }
};
class CapturingSolverForAlphas : public NOOPSolverForWorkerMaster
{
public:
    struct CapturedRow
    {
        std::vector<char> rowtype;
        std::vector<double> rhs;
        std::vector<int> mclind;
        std::vector<double> matval;
    };

    std::vector<CapturedRow> captured_rows;

    void add_rows(int newrows,
                  int newnz,
                  const char* qrtype,
                  const double* rhs,
                  const double* range,
                  const int* mstart,
                  const int* mclind,
                  const double* dmatval,
                  const std::vector<std::string>& row_names) override
    {
        CapturedRow row;
        row.rowtype = std::vector<char>(qrtype, qrtype + newrows);
        row.rhs = std::vector<double>(rhs, rhs + newrows);
        row.mclind = std::vector<int>(mclind, mclind + newnz);
        row.matval = std::vector<double>(dmatval, dmatval + newnz);
        captured_rows.push_back(row);
    }
};

class WorkerMasterAlphasFixingTest : public ::testing::Test
{
protected:
    EmptyLogManager solver_log_manager;
    std::shared_ptr<NOOPBendersProblemProvider> problem_provider =
      std::make_shared<NOOPBendersProblemProvider>();

    std::shared_ptr<WorkerMaster> make_master(int subproblems_count)
    {
        return std::make_shared<WorkerMaster>(VariableMap{},
                                              "COIN",
                                              0,
                                              subproblems_count,
                                              solver_log_manager,
                                              false,
                                              std::make_shared<xpansion::logger::Master>(),
                                              ProblemsFormat::MPS_FILE,
                                              problem_provider.get(),
                                              0.1,
                                              0.1);
    }
};

TEST_F(WorkerMasterAlphasFixingTest, NoConstraintsAddedForSingleSubproblemInCut)
{
    auto master = make_master(1);
    auto capturing_solver = std::make_shared<CapturingSolverForAlphas>();
    master->_solver = capturing_solver;
    master->set_id_single_subpb_costs_under_approx({10});

    std::map<std::string, int> problem_to_id = {{"pb0", 0}};
    std::vector<SubProblemNamesInCut> names_in_cuts = {{{"pb0", 0}}};

    master->addAlphasFixingConstraints(names_in_cuts, problem_to_id);

    EXPECT_TRUE(capturing_solver->captured_rows.empty());
}

TEST_F(WorkerMasterAlphasFixingTest, OneConstraintAddedForTwoSubproblemsInCut)
{
    auto master = make_master(2);
    auto capturing_solver = std::make_shared<CapturingSolverForAlphas>();
    master->_solver = capturing_solver;
    master->set_id_single_subpb_costs_under_approx({10, 11});

    std::map<std::string, int> problem_to_id = {{"pb0", 0}, {"pb1", 1}};
    std::vector<SubProblemNamesInCut> names_in_cuts = {{{"pb0", 0}, {"pb1", 0}}};

    master->addAlphasFixingConstraints(names_in_cuts, problem_to_id);

    ASSERT_EQ(capturing_solver->captured_rows.size(), 1u);
    const auto& row = capturing_solver->captured_rows[0];
    EXPECT_EQ(row.rowtype, std::vector<char>({'E'}));
    EXPECT_EQ(row.rhs, std::vector<double>({0.0}));
    EXPECT_EQ(row.mclind, std::vector<int>({10, 11}));
    EXPECT_EQ(row.matval, std::vector<double>({1.0, -1.0}));
}

TEST_F(WorkerMasterAlphasFixingTest, TwoConstraintsAddedForThreeSubproblemsInCut)
{
    // For a cut grouping pb0, pb1, pb2: adds alpha_0=alpha_1 and alpha_0=alpha_2
    auto master = make_master(3);
    auto capturing_solver = std::make_shared<CapturingSolverForAlphas>();
    master->_solver = capturing_solver;
    master->set_id_single_subpb_costs_under_approx({10, 11, 12});

    std::map<std::string, int> problem_to_id = {{"pb0", 0}, {"pb1", 1}, {"pb2", 2}};
    std::vector<SubProblemNamesInCut> names_in_cuts = {{{"pb0", 0}, {"pb1", 0}, {"pb2", 0}}};

    master->addAlphasFixingConstraints(names_in_cuts, problem_to_id);

    ASSERT_EQ(capturing_solver->captured_rows.size(), 2u);

    const auto& row0 = capturing_solver->captured_rows[0];
    EXPECT_EQ(row0.rowtype, std::vector<char>({'E'}));
    EXPECT_EQ(row0.rhs, std::vector<double>({0.0}));
    EXPECT_EQ(row0.mclind, std::vector<int>({10, 11}));
    EXPECT_EQ(row0.matval, std::vector<double>({1.0, -1.0}));

    const auto& row1 = capturing_solver->captured_rows[1];
    EXPECT_EQ(row1.rowtype, std::vector<char>({'E'}));
    EXPECT_EQ(row1.rhs, std::vector<double>({0.0}));
    EXPECT_EQ(row1.mclind, std::vector<int>({10, 12}));
    EXPECT_EQ(row1.matval, std::vector<double>({1.0, -1.0}));
}

TEST_F(WorkerMasterAlphasFixingTest, ConstraintsAddedPerCutIndependently)
{
    // Two cuts: first groups pb0+pb1 (adds 1 constraint), second has only pb2 (adds none)
    auto master = make_master(3);
    auto capturing_solver = std::make_shared<CapturingSolverForAlphas>();
    master->_solver = capturing_solver;
    master->set_id_single_subpb_costs_under_approx({10, 11, 12});

    std::map<std::string, int> problem_to_id = {{"pb0", 0}, {"pb1", 1}, {"pb2", 2}};
    std::vector<SubProblemNamesInCut> names_in_cuts = {{{"pb0", 0}, {"pb1", 0}}, {{"pb2", 0}}};

    master->addAlphasFixingConstraints(names_in_cuts, problem_to_id);

    ASSERT_EQ(capturing_solver->captured_rows.size(), 1u);
    const auto& row = capturing_solver->captured_rows[0];
    EXPECT_EQ(row.mclind, std::vector<int>({10, 11}));
    EXPECT_EQ(row.matval, std::vector<double>({1.0, -1.0}));
}

class WorkerMasterMock : public WorkerMaster {
public:
    using WorkerMaster::WorkerMaster; 

    void call_set_master_only_var_ids() {
        _set_master_only_var_ids();
    }

};

TEST_F(WorkerMasterTest, GetHandlesUpperBoundViolation)
{
    // Solution vector includes values for candidate variables, alpha, and single subproblem costs
    std::vector<double> solution = {10.001, 5.0, 2.0, 100.0, 50.0};
    std::vector<char> col_types = {'C', 'C', 'C', 'C', 'C'};
    std::vector<double> lbs = {0.0, 0.0, 0.0, -1e20, -1e20};
    std::vector<double> ubs = {10.0, 10.0, 10.0, 1e20, 1e20};

    double master_solution_tolerance = 0.1;
    double cut_coefficient_tolerance = 0.1;
    auto master = init_worker_master(master_solution_tolerance, cut_coefficient_tolerance);
    std::dynamic_pointer_cast<NOOPSolverForWorkerMaster>(master->_solver)
      ->setSolverBehavior(solution, col_types, lbs, ubs);
    master->get(x_out, overall_cost, single_costs, non_subpb_vars);

    EXPECT_DOUBLE_EQ(
      x_out["var1"],
      10.0); // Should be restored to UB as the tolerance is set to 0.1 in the constructor
    EXPECT_DOUBLE_EQ(x_out["var2"], 5.0);    // Should remain unchanged
    EXPECT_DOUBLE_EQ(x_out["var3"], 2.0);    // Should remain unchanged
    EXPECT_DOUBLE_EQ(overall_cost, 100.0);   // Alpha value
    EXPECT_DOUBLE_EQ(single_costs[0], 50.0); // Single subproblem cost
}

TEST_F(WorkerMasterTest, GetHandlesLowerBoundViolation)
{
    std::vector<double> solution = {1.0, -0.001, 2.0, 100.0, 50.0};
    std::vector<char> col_types = {'C', 'C', 'C', 'C', 'C'};
    std::vector<double> lbs = {0.0, 0.0, 0.0, -1e20, -1e20};
    std::vector<double> ubs = {10.0, 10.0, 10.0, 1e20, 1e20};

    double master_solution_tolerance = 0.1;
    double cut_coefficient_tolerance = 0.1;
    auto master = init_worker_master(master_solution_tolerance, cut_coefficient_tolerance);
    std::dynamic_pointer_cast<NOOPSolverForWorkerMaster>(master->_solver)
      ->setSolverBehavior(solution, col_types, lbs, ubs);
    master->get(x_out, overall_cost, single_costs,non_subpb_vars);

    EXPECT_DOUBLE_EQ(x_out["var1"], 1.0);    // Should remain unchanged
    EXPECT_DOUBLE_EQ(x_out["var2"], 0.0);    // Should be restored to LB
    EXPECT_DOUBLE_EQ(x_out["var3"], 2.0);    // Should remain unchanged
    EXPECT_DOUBLE_EQ(overall_cost, 100.0);   // Alpha value
    EXPECT_DOUBLE_EQ(single_costs[0], 50.0); // Single subproblem cost
}

TEST_F(WorkerMasterTest, GetHandlesIntegerVariables)
{
    std::vector<double> solution = {1.0, 2.499, 2.999, 100.0, 50.0};
    std::vector<char> col_types = {'C', 'I', 'I', 'C', 'C'};
    std::vector<double> lbs = {0.0, 0.0, 0.0, -1e20, -1e20};
    std::vector<double> ubs = {10.0, 10.0, 10.0, 1e20, 1e20};

    double master_solution_tolerance = 0.1;
    double cut_coefficient_tolerance = 0.1;
    auto master = init_worker_master(master_solution_tolerance, cut_coefficient_tolerance);
    std::dynamic_pointer_cast<NOOPSolverForWorkerMaster>(master->_solver)
      ->setSolverBehavior(solution, col_types, lbs, ubs);
    master->get(x_out, overall_cost, single_costs, non_subpb_vars);

    EXPECT_DOUBLE_EQ(x_out["var1"], 1.0); // Continuous - should remain unchanged
    EXPECT_DOUBLE_EQ(x_out["var2"],
                     2.499); // Integer but outside tolerance - should not round (in practice, this
                             // case should not happen, as the feasibility tolerance of the solver
                             // would guarantee a value closer to integer...)
    EXPECT_DOUBLE_EQ(x_out["var3"], 3.0);    // Integer within tolerance - should round to 3
    EXPECT_DOUBLE_EQ(overall_cost, 100.0);   // Alpha value
    EXPECT_DOUBLE_EQ(single_costs[0], 50.0); // Single subproblem cost
}

TEST_F(WorkerMasterTest, SetMasterOnlyVarIdsLogic)
{
    EmptyLogManager solver_log_manager;
    auto problem_provider = std::make_shared<NOOPBendersProblemProvider>();

    auto master = std::make_shared<WorkerMasterMock>(
        VariableMap{},
        "COIN",
        0,
        2, // subproblems_count
        solver_log_manager,
        false,
        std::make_shared<xpansion::logger::Master>(),
        ProblemsFormat::MPS_FILE,
        problem_provider.get(),
        0.1,
        std::map<int,double>{}
    );

    struct FakeSolver : public NOOPSolverForWorkerMaster {
        int get_ncols() const override { return 6; }
    };
    master->_solver = std::make_shared<FakeSolver>();

    master->_name_to_id = {{"var0", 0}, {"var1", 1}, {"var2", 2}};
    master->_id_master_only_vars.clear();

    master->call_set_master_only_var_ids();

    std::vector<int> expected_empty{};
    EXPECT_EQ(master->_id_master_only_vars, expected_empty);

    master->_name_to_id = {{"var0", 0}, {"var1", 1}}; 
    master->_id_master_only_vars.clear();

    master->call_set_master_only_var_ids();

    std::vector<int> expected{2};
    EXPECT_EQ(master->_id_master_only_vars, expected);
}

TEST_F(WorkerMasterTest, AddSubproblemCutAppliesRoundingOnCoeffs)
{
    double master_solution_tolerance = 0.1;
    double cut_coefficient_tolerance = 0.1;
    auto master = init_worker_master(master_solution_tolerance, cut_coefficient_tolerance);

    Point subgradient;
    subgradient["var1"] = -5e-3; 
    subgradient["var2"] = -4e-2;
    subgradient["var3"] = -3e-1;

    Point x_cut;
    x_cut["var1"] = 1.0;
    x_cut["var2"] = 10.0;
    x_cut["var3"] = 100.0;

    double subproblem_cost = 10.0;

    master->addSubproblemCut(0, subgradient, x_cut, subproblem_cost);
    // cut is -theta_i + subgradient.x <= -subproblem_cost + subgradient.x_cut (in the solver)
    // i.e. theta_i >= subproblem_cost + subgradient.(x - x_cut) (human form)

    auto mockSolver = std::dynamic_pointer_cast<NOOPSolverForWorkerMaster>(master->_solver);

    EXPECT_EQ(mockSolver->last_rowrhs.size(), 1);
    EXPECT_EQ(mockSolver->last_rowrhs[0], -40.405);

    EXPECT_EQ(mockSolver->last_matval.size(),4);
    EXPECT_EQ(mockSolver->last_matval[0], 0.0);
    EXPECT_EQ(mockSolver->last_matval[1], 0.0);
    EXPECT_EQ(mockSolver->last_matval[2], -0.3);
    EXPECT_EQ(mockSolver->last_matval[3], -1);

}

