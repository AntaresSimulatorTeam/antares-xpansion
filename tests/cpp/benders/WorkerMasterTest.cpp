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

    std::filesystem::path provide_file_path() const
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

    std::shared_ptr<WorkerMaster> init_worker_master(double master_solution_tolerance,
                                                     double cut_coefficient_tolerance) const
    {
        auto test_solver = std::make_shared<NOOPSolverForWorkerMaster>();
        EmptyLogManager solver_log_manager;
        auto problem_provider = std::make_shared<NOOPBendersProblemProvider>();
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
                                                     cut_coefficient_tolerance);
        master->_solver = test_solver;
        master->_id_to_name = {{0, "var1"}, {1, "var2"}, {2, "var3"}};
        master->set_id_alpha(3);
        master->set_id_single_subpb_costs_under_approx({4});
        return master;
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
    master->get(x_out, overall_cost, single_costs);

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
    master->get(x_out, overall_cost, single_costs);

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
    master->get(x_out, overall_cost, single_costs);

    EXPECT_DOUBLE_EQ(x_out["var1"], 1.0); // Continuous - should remain unchanged
    EXPECT_DOUBLE_EQ(x_out["var2"],
                     2.499); // Integer but outside tolerance - should not round (in practice, this
                             // case should not happen, as the feasibility tolerance of the solver
                             // would guarantee a value closer to integer...)
    EXPECT_DOUBLE_EQ(x_out["var3"], 3.0);    // Integer within tolerance - should round to 3
    EXPECT_DOUBLE_EQ(overall_cost, 100.0);   // Alpha value
    EXPECT_DOUBLE_EQ(single_costs[0], 50.0); // Single subproblem cost
}
