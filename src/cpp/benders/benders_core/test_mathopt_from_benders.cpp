#include <iostream>
#include <ortools/math_opt/cpp/math_opt.h>

using namespace operations_research::math_opt;

int main()
{
    Model model("BendersTestLP");

    std::cout << "model creating " << std::endl;
    Variable x = model.AddContinuousVariable(0.0, 10.0, "x");
    Variable y = model.AddContinuousVariable(0.0, 10.0, "y");
    std::cout << "variable add " << std::endl;

    LinearConstraint c1 = model.AddLinearConstraint(x + 2 * y <= 14, "capacity");
    LinearConstraint c2 = model.AddLinearConstraint(3 * x + y <= 14, "demand");

    std::cout << "constraint  add " << std::endl;

    model.Maximize(x + y);

    std::cout << "maximize called " << std::endl;

    SolveArguments args;
    args.parameters.enable_output = true;

    auto result = Solve(model, SolverType::kGlop, args);
    if (!result.ok())
    {
        std::cerr << "Solve failed: " << result.status() << "\n";
        return 1;
    }

    const SolveResult& sol = *result;

    std::cout << "Termination: " << sol.termination << "\n";

    if (sol.termination.IsOptimal())
    {
        std::cout << "Optimal objective: " << sol.objective_value() << "\n";
        std::cout << "x = " << sol.variable_values().at(x) << "\n";
        std::cout << "y = " << sol.variable_values().at(y) << "\n";

        if (sol.has_dual_feasible_solution())
        {
            std::cout << "Dual of capacity: " << sol.dual_values().at(c1) << "\n";
            std::cout << "Dual of demand:   " << sol.dual_values().at(c2) << "\n";
            std::cout << "Reduced cost x:   " << sol.reduced_costs().at(x) << "\n";
            std::cout << "Reduced cost y:   " << sol.reduced_costs().at(y) << "\n";
        }
    }
    else
    {
        std::cerr << "Not optimal: " << sol.termination << "\n";
        return 1;
    }

    // Test modifying the model and re-solving
    model.set_upper_bound(c1, 20.0);
    model.set_objective_coefficient(x, 3.0);

    auto result2 = Solve(model, SolverType::kGlop, args);
    if (result2.ok() && result2->termination.IsOptimal())
    {
        std::cout << "\nAfter modification:\n";
        std::cout << "Optimal objective: " << result2->objective_value() << "\n";
        std::cout << "x = " << result2->variable_values().at(x) << "\n";
        std::cout << "y = " << result2->variable_values().at(y) << "\n";
    }

    std::cout << "\nMathOpt from benders_core: OK\n";
    return 0;
}
