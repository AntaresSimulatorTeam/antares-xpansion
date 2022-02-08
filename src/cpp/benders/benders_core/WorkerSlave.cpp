#include "WorkerSlave.h"

#include "solver_utils.h"

/*!
 *  \brief Constructor of a Worker Slave
 *
 *  \param variable_map : Map of linking each variable of the problem to its id
 *
 *  \param problem_name : Name of the problem
 *
 */
WorkerSlave::WorkerSlave(Str2Int const &variable_map, std::string const &path_to_mps, double const &slave_weight, const std::string &solver_name, const int log_level)
{

	init(variable_map, path_to_mps, solver_name, log_level);

	int mps_ncols(_solver->get_ncols());
	DblVector obj_func_coeffs(mps_ncols);
	IntVector sequence(mps_ncols);
	for (int i = 0; i < mps_ncols; ++i)
	{
		sequence[i] = i;
	}
	solver_get_obj_func_coeffs(_solver, obj_func_coeffs, 0, mps_ncols - 1);
	for (auto &c : obj_func_coeffs)
	{
		c *= slave_weight;
	}
	_solver->chg_obj(sequence, obj_func_coeffs);
}

/*!
 *  \brief Fix a set of variables to constant in a problem
 *
 *  Method to set variables in a problem by fixing their bounds
 *
 *  \param x0 : Set of variables to fix
 */
void WorkerSlave::fix_to(Point const &x0)
{
	int nbnds((int)_name_to_id.size());
	std::vector<int> indexes(nbnds);
	std::vector<char> bndtypes(nbnds, 'B');
	std::vector<double> values(nbnds);

	int i(0);
	for (auto const &kvp : _id_to_name)
	{
		indexes[i] = kvp.first;
		values[i] = x0.find(kvp.second)->second;
		++i;
	}

	solver_chgbounds(_solver, indexes, bndtypes, values);
}

/*!
 *  \brief Get LP solution value of a problem
 *
 *  \param s : Empty point which receives the solution
 */
void WorkerSlave::get_subgradient(Point &s)
{
	s.clear();
	std::vector<double> ptr(_solver->get_ncols());
	solver_getlpreducedcost(_solver, ptr);
	for (auto const &kvp : _id_to_name)
	{
		s[kvp.second] = +ptr[kvp.first];
	}
}

/*!
 *  \brief Get simplex basis of a problem
 *
 *  Method to store simplex basis of a problem, and build the distance matrix
 */
SimplexBasis WorkerSlave::get_basis()
{
	IntVector cstatus(_solver->get_ncols());
	IntVector rstatus(_solver->get_nrows());
	solver_getbasis(_solver, rstatus, cstatus);
	return std::make_pair(rstatus, cstatus);
}
