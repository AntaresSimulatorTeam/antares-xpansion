#include "WorkerSlave.h"
#include "launcher.h"

#include "ortools_utils.h"

WorkerSlave::WorkerSlave() {

}


/*!
*  \brief Constructor of a Worker Slave
*
*  \param variable_map : Map of linking each variable of the problem to its id
*
*  \param problem_name : Name of the problem
*
*/
WorkerSlave::WorkerSlave(Str2Int const & variable_map, std::string const & path_to_mps, double const & slave_weight, BendersOptions const & options) {
	init(variable_map, path_to_mps);

	int mps_ncols(_solver->NumVariables());
	DblVector o_l;
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	ORTgetobj(*_solver, o_l, 0, mps_ncols - 1);
	//std::cout << "slave_weight : " << slave_weight << std::endl;
	for (auto & c : o_l) {
		c *= slave_weight;
	}
	ORTchgobj(*_solver, sequence, o_l);
	// XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);

	//IntVector scols;
	//for (auto const & x : _id_to_name) {
	//	scols.push_back(x.first);
	//}
	//int ncols;
	//int nrows;
	//XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	//XPRSgetintattrib(_xprs, XPRS_COLS, &nrows);
	//XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	//XPRSloadsecurevecs(_xprs, 0, scols.size(), NULL, scols.data());
	//XPRSsetintcontrol(_xprs, XPRS_LPITERLIMIT, 0);
	//XPRSlpoptimize(_xprs, "");
	//double objrhs(0);
	//XPRSgetdblattrib(_xprs, XPRS_OBJRHS, &objrhs);
	//IntVector colmap(ncols);
	//XPRSgetpresolvemap(_xprs, NULL, colmap.data());
	//for (int i(0); i < ncols; ++i) {
	//	//if(_id_to_name.find(colmap[i]) != _id_to_name.end())
	//	//std::cout << i << "   |   " << colmap[i] << std::endl;
	//}
	//StandardLp datapre(_xprs);
	//IntVector precolstatus(ncols);
	//IntVector prerowstatus(nrows);
	//XPRSgetpresolvebasis(_xprs, prerowstatus.data(), precolstatus.data());
	//XPRSprob prexp;
	//XPRScreateprob(&prexp);
	//XPRSsetcbmessage(prexp, optimizermsg, this);
	//XPRSsetintcontrol(prexp, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	//XPRSsetintcontrol(prexp, XPRS_THREADS, 1);
	//XPRSloadlp(prexp, "prexp", 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	//datapre.append_in(prexp);
	//IntVector minus_one(1, -1);
	//std::cout << "objrhs : " << objrhs << std::endl;
	//XPRSchgobj(prexp, 1, minus_one.data(), &objrhs);
	//XPRSloadbasis(prexp, prerowstatus.data(), precolstatus.data());
	//std::cout << "solving prexp" << std::endl;
	//XPRSlpoptimize(prexp, "");
	//std::cout << "solving xprs" << std::endl;
	//XPRSsetintcontrol(_xprs, XPRS_LPITERLIMIT, 2147483645);
	//XPRSlpoptimize(_xprs, "");
	//std::exit(0);
}
WorkerSlave::~WorkerSlave() {

}

/*!
*  \brief Fix a set of variables to constant in a problem
*
*  Method to set variables in a problem by fixing their bounds
*
*  \param x0 : Set of variables to fix
*/
void WorkerSlave::fix_to(Point const & x0) {
	int nbnds((int)_name_to_id.size());
	std::vector<int> indexes(nbnds);
	std::vector<char> bndtypes(nbnds, 'B');
	std::vector<double> values(nbnds);

	int i(0);
	for (auto const & kvp : _id_to_name) {
		indexes[i] = kvp.first;
		values[i] = x0.find(kvp.second)->second;
		++i;
	}

	ORTchgbounds(*_solver, indexes, bndtypes, values);
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	std::vector<double> ptr;
	ORTgetlpreducedcost(*_solver, ptr);
	for (auto const & kvp : _id_to_name) {
		s[kvp.second] = +ptr[kvp.first];
	}

}

/*!
*  \brief Get simplex basis of a problem
*
*  Method to store simplex basis of a problem, and build the distance matrix
*/
SimplexBasis WorkerSlave::get_basis() {
	IntVector cstatus;
	IntVector rstatus;
	ORTgetbasis(*_solver, rstatus, cstatus);
	return std::make_pair(rstatus, cstatus);
}

