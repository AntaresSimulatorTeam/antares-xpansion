//
// Created by s90365 on 23/08/2021.
//
#include <solver_utils.h>
#include "MasterProblemBuilder.h"

std::shared_ptr<SolverAbstract> MasterProblemBuilder::build(const std::string& solverName, const std::vector<ActiveLink>& links)
{
	SolverFactory factory;
	auto math_problem = factory.create_solver(solverName);

	return math_problem;
}
