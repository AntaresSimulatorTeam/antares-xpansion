#include "Worker.h"


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

void XPRS_CC optimizermsg(XPRSprob prob, void* worker, const char *sMsg, int nLen,
	int nMsglvl) {
	Worker * ptr = NULL;
	if (worker != NULL)ptr = (Worker*)worker;
	switch (nMsglvl) {

		/* Print Optimizer error messages and warnings */
	case 4: /* error */
	case 3: /* warning */
	case 2: /* dialogue */
	case 1: /* information */
		if (ptr != NULL) {
			for (auto const & stream : ptr->stream())
				*stream << sMsg << std::endl;
		}
		else {
			std::cout << sMsg << std::endl;
		}
		break;
		/* Exit and flush buffers */
	default:
		fflush(NULL);
		break;
	}
}

/************************************************************************************\
* Name:         errormsg                                                             *
* Purpose:      Display error information about failed subroutines.                  *
* Arguments:    const char *sSubName    Subroutine name                              *
*               int nLineNo             Line number                                  *
*               int nErrCode            Error code                                   *
* Return Value: None                                                                 *
\************************************************************************************/

void errormsg(XPRSprob & _xprs, const char *sSubName, int nLineNo, int nErrCode) {
	int nErrNo; /* Optimizer error number */
				/* Print error message */
	printf("The subroutine %s has failed on line %d\n", sSubName, nLineNo);

	/* Append the error code if it exists */
	if (nErrCode != -1)
		printf("with error code %d.\n\n", nErrCode);

	/* Append Optimizer error number, if available */
	if (nErrCode == 32) {
		XPRSgetintattrib(_xprs, XPRS_ERRORCODE, &nErrNo);
		printf("The Optimizer eror number is: %d\n\n", nErrNo);
	}

	/* Free memory close files and exit */
	XPRSdestroyprob(_xprs);
	XPRSfree();
	exit(nErrCode);
}
Worker::Worker() {

}

Worker::~Worker() {
	
}

/*!
*  \brief Free the problem
*/
void Worker::free() {
	XPRSdestroyprob(_xprs);
}

/*!
*  \brief Return the optimal value of a problem
*
*  \param lb : double which receives the optimal value
*/
void Worker::get_value(double & lb) {
	XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &lb);
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
	XPRScreateprob(&_xprs);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
	XPRSsetintcontrol(_xprs, XPRS_THREADS, 1);
	XPRSsetcbmessage(_xprs, optimizermsg, this);
	XPRSreadprob(_xprs, path_to_mps.c_str(), "");

	//std::ifstream file(_path_to_mapping.c_str());
	_name_to_id = variable_map;
	for(auto const & kvp : variable_map) {
		_id_to_name[kvp.second] = kvp.first;
	}
	_is_master = false;
}

StrVector XPRS_LP_STATUS = {
	"XPRS_LP_UNSTARTED",
	"XPRS_LP_OPTIMAL",
	"XPRS_LP_INFEAS",
	"XPRS_LP_CUTOFF",
	"XPRS_LP_UNFINISHED",
	"XPRS_LP_UNBOUNDED",
	"XPRS_LP_CUTOFF_IN_DUAL",
	"XPRS_LP_UNSOLVED",
	"XPRS_LP_NONCONVEX"
};

/*!
*  \brief Method to solve a problem
*
*  \param lp_status : problem status after optimization
*/
void Worker::solve(int & lp_status) {

	int status(0);

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
	status = XPRSlpoptimize(_xprs, "");
	
	XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &lp_status);
	if (lp_status != XPRS_LP_OPTIMAL) {
		std::cout << "lp_status is : " << lp_status << std::endl;
		std::stringstream buffer;
		
		buffer << _path_to_mps << "_lp_status_";
		buffer << XPRS_LP_STATUS[lp_status];
		buffer<< ".mps";
		std::cout << "lp_status is : " << XPRS_LP_STATUS[lp_status] << std::endl;
		std::cout << "written in " << buffer.str() << std::endl;
		XPRSwriteprob(_xprs, buffer.str().c_str(), "x");
		std::exit(0);
	}
	else if (status) {
		std::cout << "Worker::solve() status " << status<<", "<<_path_to_mps << std::endl;
		
	}
}

/*!
*  \brief Get the number of iteration needed to solve a problem
*
*  \param result : result
*/
void Worker::get_simplex_ite(int & result) {
	XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
}