#include <cmath>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/WorkerMaster.h"

class TestNOOPSolver: public NOOPSolver
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

    int get_ncols() const override
    {
        return solution.size();
    }

    int get_n_integer_vars() const override
    {
        return 0;
    }

    void get_lp_sol(double* sol, double*, double*) const override
    {
        std::copy(solution.begin(), solution.end(), sol);
    }

    void get_col_type(char* coltype, int, int) const override
    {
        std::copy(col_types.begin(), col_types.end(), coltype);
    }

    void get_lb(double* lb, int, int) const override
    {
        std::copy(lbs.begin(), lbs.end(), lb);
    }

    void get_ub(double* ub, int, int) const override
    {
        std::copy(ubs.begin(), ubs.end(), ub);
    }

private:
    std::vector<double> solution;
    std::vector<char> col_types;
    std::vector<double> lbs;
    std::vector<double> ubs;
};

class NOOPWorker: public Worker
{
public:
    NOOPWorker():
        Worker({}, "", Logger(), 0.01)
    {
    }

    void init(const std::string& solver_name,
              int log_level,
              const SolverLogManager& solver_log_manager,
              ProblemsFormat format) override
    {
        // Do nothing - avoid actual solver/MPS initialization
    }
};

// Warning : Diamond problem as TestableWorkerMaster inherits from WorkerMaster and NOOPWorker, that
// both inherit from Worker. But we want to override the Worker init method to ease testing...
class TestableWorkerMaster: public WorkerMaster, private NOOPWorker
{
public:
    TestableWorkerMaster():
        WorkerMaster(
          {},
          "",
          "",
          0,
          1,
          solver_log_manager_, // Use the static instance automatically initialized beforehand
          false,
          Logger(),
          ProblemsFormat::MPS_FILE,
          0.01,
          0.01),
        test_solver(std::make_shared<TestNOOPSolver>())
    {
        WorkerMaster::_solver = test_solver;
        WorkerMaster::_id_to_name = {{0, "var1"}, {1, "var2"}, {2, "var3"}};
        set_id_alpha(3);
        set_id_single_subpb_costs_under_approx({4});
    }

    void setNOOPSolverBehavior(const std::vector<double>& solution,
                               const std::vector<char>& col_types,
                               const std::vector<double>& lbs,
                               const std::vector<double>& ubs)
    {
        test_solver->setSolverBehavior(solution, col_types, lbs, ubs);
    }

protected:
    // Override Worker::init with NOOPWorker's implementation
    // does not work as init is called from the WorkerMaster class inside WorkerMaster
    // constructor..., so never call this init...
    void init(const std::string& solver_name,
              int log_level,
              const SolverLogManager& solver_log_manager,
              ProblemsFormat format) override
    {
        NOOPWorker::init(solver_name, log_level, solver_log_manager, format);
    }

private:
    std::shared_ptr<TestNOOPSolver> test_solver;
    static SolverLogManager solver_log_manager_; // Static so it's initialized before use
};

// Use a static member as in TEST_F we want to can only call the default constructor of
// WorkerMasterTest. therefore we cannot pass solver_log_manager as argument to TestableWorkerMaster
// constructor. using a non static attribute solver_log_manager will result in a -Wreoder warning
// when building TestableWorkerMaster. As the static member is built before calling the constructor,
// we do not have the warning anymore. there may be better designs though.
SolverLogManager TestableWorkerMaster::solver_log_manager_;

class WorkerMasterTest: public ::testing::Test
{
protected:
    TestableWorkerMaster master;
    Point x_out;
    double overall_cost;
    DblVector single_costs{0.0};
};

TEST_F(WorkerMasterTest, GetHandlesUpperBoundViolation)
{
    // Solution vector includes values for candidate variables, alpha, and single subproblem costs
    std::vector<double> solution = {10.001, 5.0, 2.0, 100.0, 50.0};
    std::vector<char> col_types = {'C', 'C', 'C', 'C', 'C'};
    std::vector<double> lbs = {0.0, 0.0, 0.0, -1e20, -1e20};
    std::vector<double> ubs = {10.0, 10.0, 10.0, 1e20, 1e20};

    master.setNOOPSolverBehavior(solution, col_types, lbs, ubs);
    master.get(x_out, overall_cost, single_costs);

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

    master.setNOOPSolverBehavior(solution, col_types, lbs, ubs);
    master.get(x_out, overall_cost, single_costs);

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

    master.setNOOPSolverBehavior(solution, col_types, lbs, ubs);
    master.get(x_out, overall_cost, single_costs);

    EXPECT_DOUBLE_EQ(x_out["var1"], 1.0); // Continuous - should remain unchanged
    EXPECT_DOUBLE_EQ(x_out["var2"],
                     2.499); // Integer but outside tolerance - should not round (in practice, this
                             // case should not happen, as the feasibility tolerance of the solver
                             // would guarantee a value closer to integer...)
    EXPECT_DOUBLE_EQ(x_out["var3"], 3.0);    // Integer within tolerance - should round to 3
    EXPECT_DOUBLE_EQ(overall_cost, 100.0);   // Alpha value
    EXPECT_DOUBLE_EQ(single_costs[0], 50.0); // Single subproblem cost
}
