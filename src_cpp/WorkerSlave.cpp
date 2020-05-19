#include "WorkerSlave.h"
#include "launcher.h"

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
	if (options.XPRESS_TRACE == 2 || options.XPRESS_TRACE == 3) {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	}
	else {
		XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	}
	int mps_ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &mps_ncols);
	DblVector o(mps_ncols, 0);
	IntVector sequence(mps_ncols);
	for (int i(0); i < mps_ncols; ++i) {
		sequence[i] = i;
	}
	XPRSgetobj(_xprs, o.data(), 0, mps_ncols - 1);
	//std::cout << "slave_weight : " << slave_weight << std::endl;
	for (auto & c : o) {
		c *= slave_weight;
	}
	XPRSchgobj(_xprs, mps_ncols, sequence.data(), o.data());
	XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);

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
*  \brief Write in a problem in an lp file 
*
* Method to write a problem in an lp file
*
*  \param it : id of the problem
*/
void WorkerSlave::write(int it) {
	std::stringstream name;
	name << "slave_" << it << ".lp";
	XPRSwriteprob(_xprs, name.str().c_str(), "l");
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

	XPRSchgbounds(_xprs, nbnds, indexes.data(), bndtypes.data(), values.data());
}

/*!
*  \brief Get LP solution value of a problem
*
*  \param s : Empty point which receives the solution
*/
void WorkerSlave::get_subgradient(Point & s) {
	s.clear();
	int ncols;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	std::vector<double> ptr(ncols, 0);
	XPRSgetlpsol(_xprs, NULL, NULL, NULL, ptr.data());
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
	int ncols;
	int nrows;
	IntVector cstatus;
	IntVector rstatus;
	XPRSgetintattrib(_xprs, XPRS_COLS, &ncols);
	XPRSgetintattrib(_xprs, XPRS_ROWS, &nrows);
	cstatus.resize(ncols);
	rstatus.resize(nrows);
	XPRSgetbasis(_xprs, rstatus.data(), cstatus.data());
	return std::make_pair(rstatus, cstatus);
}

