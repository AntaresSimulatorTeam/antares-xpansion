#include <cmath>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "NOOPSolver.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include "antares-xpansion/benders/logger/Master.h"

class NOOPSolverForSubproblemWorker: public NOOPSolverForWorker
{
public:
    void setSolverBehavior(const std::vector<double>& red_costs,
                           const std::vector<char>& types,
                           const std::vector<double>& lower_bounds,
                           const std::vector<double>& upper_bounds)
    {
        reduced_costs = red_costs;
        col_types = types;
        lbs = lower_bounds;
        ubs = upper_bounds;
    }

    int get_ncols() const override
    {
        return reduced_costs.size();
    }

    int get_n_integer_vars() const override
    {
        return 0;
    }

    void get_lp_sol(double*, double*, double* red_costs) const override
    {
        std::copy(reduced_costs.begin(), reduced_costs.end(), red_costs);
    }

private:
    std::vector<double> reduced_costs;
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

class EmptyLogManager: public SolverLogManager
{
public:
    SolverLogManager& operator=(const SolverLogManager& other) override
    {
        return *this;
    }

    void init() override
    {
    }

    ~EmptyLogManager() = default;
};

class SubproblemWorkerTest: public ::testing::Test

{
public:
    SubproblemWorkerTest() = default;

protected:
    Point subgradient;

    std::shared_ptr<SubproblemWorker> init_subproblem_worker(double cut_coefficient_tolerance) const
    {
        auto test_solver = std::make_shared<NOOPSolverForSubproblemWorker>();
        EmptyLogManager solver_log_manager;
        auto problem_provider = std::make_shared<NOOPBendersProblemProvider>();
        auto subproblem = std::make_shared<SubproblemWorker>(
          VariableMap{},
          0,
          "COIN",
          1,
          solver_log_manager,
          std::make_shared<xpansion::logger::Master>(),
          ProblemsFormat::MPS_FILE,
          problem_provider.get(),
          cut_coefficient_tolerance);
        subproblem->_solver = test_solver;
        subproblem->_id_to_name = {{2, "var1"}, {3, "var2"}, {4, "var3"}};
        return subproblem;
    }
};

TEST_F(SubproblemWorkerTest, RoundSubgradientIfWithinTolerance)
{
    // Solution vector includes values for candidate variables, alpha, and single subproblem costs
    std::vector<double> reduced_cots = {0, 0.01, -1.0, 0.01, -0.05};
    std::vector<char> col_types;
    std::vector<double> lbs;
    std::vector<double> ubs;

    double cut_coefficient_tolerance = 0.1;
    auto subproblem = init_subproblem_worker(cut_coefficient_tolerance);
    std::dynamic_pointer_cast<NOOPSolverForSubproblemWorker>(subproblem->_solver)
      ->setSolverBehavior(reduced_cots, col_types, lbs, ubs);
    subproblem->get_subgradient(subgradient);

    EXPECT_TRUE(subgradient["var1"] == -1.0);
    EXPECT_TRUE(subgradient["var2"] == 0);
    EXPECT_TRUE(subgradient["var3"] == 0);
}
