#include "Worker.h"

#include "ortools_utils.h"


std::list<std::ostream *> & Worker::stream() {
	return _stream;
}

/************************************************************************************\
* Name:         optimizermsg                                                         *
* Purpose:      Display Optimizer error messages and warnings.                       *
* Arguments:    const char *sMsg    Message string                                   *
*               int nLen            Message length                                   *
*               int nMsgLvl         Message type                                     *
* Return Value: None.                                                                *
\************************************************************************************/

// void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen,
// 	int nMsglvl) {
// 	Worker * ptr = NULL;
// 	if (worker != NULL) ptr = static_cast<Worker*>(worker);
// 	switch (nMsglvl) {

// 		/* Print Optimizer error messages and warnings */
// 	case 4: /* error */
// 	case 3: /* warning */
// 	case 2: /* dialogue */
// 	case 1: /* information */
// 		if (ptr != NULL) {
// 			for (auto const & stream : ptr->stream())
// 				*stream << sMsg << std::endl;
// 		}
// 		else {
// 			std::cout << sMsg << std::endl;
// 		}
// 		break;
// 		/* Exit and flush buffers */
// 	default:
// 		fflush(NULL);
// 		break;
// 	}
// }

Worker::Worker()
	: _is_master(false)
	, _solver(nullptr)
{
}

Worker::~Worker() {

}

/*!
*  \brief Free the problem
*/
void Worker::free() {
	delete _solver;
	_solver = nullptr;
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void Worker::get_value(double & lb) {
	lb = _solver->Objective().Value();
}

/*!
*  \brief Initialization of a problem
*
*  \param variable_map : map linking each problem name to its variables and their ids
*
*  \param problem_name : name of the problem
*/
void Worker::init(Str2Int const & variable_map, std::string const & path_to_mps) {
	_path_to_mps = path_to_mps;
	_stream.push_back(&std::cout);

	if (_is_master)
	{
		_solver = new operations_research::MPSolver(path_to_mps, ORTOOLS_MIP_SOLVER_TYPE);
	}
	else
	{
		_solver = new operations_research::MPSolver(path_to_mps, ORTOOLS_LP_SOLVER_TYPE);
	}
	// _solver->EnableOutput();
	_solver->SetNumThreads(1);
	ORTreadmps(*_solver, path_to_mps);

	//std::ifstream file(_path_to_mapping.c_str());
	bool error_l = false;
	for(auto const & kvp : variable_map) {
		operations_research::MPVariable const * const var_l = _solver->LookupVariableOrNull(kvp.first);
		if ( var_l != nullptr)
		{
			_id_to_name[var_l->index()] = kvp.first;
			_name_to_id[kvp.first] = var_l->index();
		}
		else
		{
			error_l = true;
			std::cout << "\nERROR : missing variable " << kvp.first << " in " << path_to_mps;
		}
	}
	if(error_l)	std::exit(1);
}

StrVector ORT_LP_STATUS = {
	"ORT_OPTIMAL",
    "ORT_FEASIBLE",
    "ORT_INFEASIBLE",
    "ORT_UNBOUNDED",
    "ORT_ABNORMAL",
    "ORT_MODEL_INVALID",
    "ORT_NOT_SOLVED"
};

/*!
*  \brief Method to solve a problem
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve(int & lp_status) {

	//int initial_rows(0);
	//int presolved_rows(0);
	//if (_is_master) {
	//	XPRSgetintattrib(_xprs, XPRS_ROWS, &initial_rows);

	//	XPRSsetintcontrol(_xprs, XPRS_LPITERLIMIT, 0);
	//	XPRSsetintcontrol(_xprs, XPRS_BARITERLIMIT, 0);

	//	status = XPRSlpoptimize(_xprs, "");

	//	XPRSgetintattrib(_xprs, XPRS_ROWS, &presolved_rows);

	//	XPRSsetintcontrol(_xprs, XPRS_LPITERLIMIT, 2147483645);
	//	XPRSsetintcontrol(_xprs, XPRS_BARITERLIMIT, 500);
	//}

	lp_status = _solver->Solve();

	if (lp_status != operations_research::MPSolver::OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;

		buffer << _path_to_mps << "_lp_status_";
		buffer << ORT_LP_STATUS[lp_status];
		buffer<< ".mps";
		std::cout << "lp_status is : " << ORT_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		ORTwritemps(*_solver, buffer.str());
		std::exit(1);
	}
	else {//@FIXME conformity : replace with equivalent to XPRS_LP_UNSTARTED but useless
		//std::cout << "Worker::solve() status " << lp_status<<", "<<_path_to_mps << std::endl;
	}
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	result = _solver->iterations();
}
