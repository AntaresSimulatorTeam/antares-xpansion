#ifdef CPLEX
#include "SolverCplex.h"
#endif
#ifdef XPRESS
#include "SolverXpress.h"
#endif
#ifdef COIN_OR
#include "SolverCbc.h"
#include "SolverClp.h"
#endif

#include "multisolver_interface/SolverFactory.h"
SolverFactory::SolverFactory()
{
	_available_solvers.clear();
#ifdef CPLEX
	_available_solvers.insert(d);
#endif
#ifdef XPRESS
	_available_solvers.insert(SOLVER_TYPE::XPRESS);
#endif
#ifdef COIN_OR
	_available_solvers.insert(SOLVER_TYPE::CLP);
	_available_solvers.insert(SOLVER_TYPE::CBC);
#endif
}

SolverAbstract::Ptr SolverFactory::create_solver(const std::string solver_name)
{

	SolverAbstract::Ptr solver;
	if (solver_name == "")
	{
		throw InvalidSolverNameException(solver_name);
	}
	SOLVER_TYPE a_solver_type = str_to_solver_type(solver_name);
	if (_available_solvers.find(a_solver_type) != _available_solvers.end())
	{
		switch (a_solver_type)
		{
		case SOLVER_TYPE::CPLEX:
			solver = std::make_shared<SolverCplex>();
			break;
		case SOLVER_TYPE::XPRESS:
			solver = std::make_shared<SolverXpress>();
			break;
		case SOLVER_TYPE::CLP:
			solver = std::make_shared<SolverClp>();
			break;
		case SOLVER_TYPE::CBC:
			solver = std::make_shared<SolverCbc>();
			break;
		default:
			throw InvalidSolverNameException(solver_name);
			break;
		}
	}
	else
	{
		throw InvalidSolverNameException(solver_name);
	}

	return solver;
}

SolverAbstract::Ptr SolverFactory::create_solver(SolverAbstract::Ptr to_copy)
{
	SolverAbstract::Ptr solver;
	std::string solver_name = to_copy->get_solver_name();

	if (solver_name == "")
	{
		throw InvalidSolverNameException(solver_name);
	}

	SOLVER_TYPE a_solver_type = str_to_solver_type(solver_name);
	if (_available_solvers.find(a_solver_type) != _available_solvers.end())
	{
		switch (a_solver_type)
		{
		case SOLVER_TYPE::CPLEX:
			solver = std::make_shared<SolverCplex>(to_copy);
			break;
		case SOLVER_TYPE::XPRESS:
			solver = std::make_shared<SolverXpress>(to_copy);
			break;
		case SOLVER_TYPE::CLP:
			solver = std::make_shared<SolverClp>(to_copy);
			break;
		case SOLVER_TYPE::CBC:
			solver = std::make_shared<SolverCbc>(to_copy);
			break;
		default:
			throw InvalidSolverNameException(solver_name);
			break;
		}
	}
	else
	{
		throw InvalidSolverNameException(solver_name);
	}

	return solver;
}
const std::vector<std::string> &SolverFactory::get_solvers_list() const
{
	return _available_solvers;
}