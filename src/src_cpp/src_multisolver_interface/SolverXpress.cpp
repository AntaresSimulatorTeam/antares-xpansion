#include "SolverXpress.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor    --------------------------------
*************************************************************************************************/
int SolverXpress::_NumberOfProblems = 0;

SolverXpress::SolverXpress() {
	int status = 0;
	if (_NumberOfProblems == 0) {
		status = XPRSinit(NULL);
		zero_status_check(status, "intialize XPRESS environment");
	}

	_NumberOfProblems += 1;
	_xprs = NULL;
}

SolverXpress::SolverXpress(const std::string& name, const SolverAbstract::Ptr fictif) {
	SolverXpress();
	std::cout << "Copy constructor of XPRESS : TO DO WHEN NEEDED" << std::endl;
	std::exit(0);
}

SolverXpress::~SolverXpress() {
	_NumberOfProblems -= 1;
	free();

	if (_NumberOfProblems == 0) {
		int status = XPRSfree();
		zero_status_check(status, "free XPRESS environment");
		std::cout << "Closing XPRESS environment." << std::endl;
	}
}

int SolverXpress::get_number_of_instances()
{
	return _NumberOfProblems;
}

/*************************************************************************************************
-----------------------------------    Output stream management    ------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments    ---------------
*************************************************************************************************/
void SolverXpress::init() {
	int status = XPRScreateprob(&_xprs);
	zero_status_check(status, "create XPRESS problem");
}

void SolverXpress::free() {
	int status = XPRSdestroyprob(_xprs);
	_xprs = NULL;
	zero_status_check(status, "destroy XPRESS problem");
}


/*************************************************************************************************
-------------------------------    Reading & Writing problems    -------------------------------
*************************************************************************************************/
void SolverXpress::write_prob(const char* name, const char* flags) const{
	std::string nFlags = "";
	if (std::string(flags) == "LP") {
		nFlags = "-l";
	}
	int status = XPRSwriteprob(_xprs, name, nFlags.c_str());
	zero_status_check(status, "write problem");
}

void SolverXpress::read_prob(const char* prob_name, const char* flags){
	std::string xprs_flags = "";
	if (std::string(flags) == "LP") {
		xprs_flags = "l";
	}

	// To delete obj from rows when reading prob
	int keeprows(0);
	int status = XPRSgetintcontrol(_xprs, XPRS_KEEPNROWS, &keeprows);
	zero_status_check(status, "get XPRS_KEEPNROWS");

	if (keeprows != -1) {
		status = XPRSsetintcontrol(_xprs, XPRS_KEEPNROWS, -1);
		zero_status_check(status, "set XPRS_KEEPNROWS to -1");
	}
	
	status = XPRSreadprob(_xprs, prob_name, xprs_flags.c_str());
	zero_status_check(status, "read problem");
}

void SolverXpress::copy_prob(const SolverAbstract::Ptr fictif_solv){
	std::cout << "Copy XPRESS problem : TO DO WHEN NEEDED" << std::endl;
	std::exit(0);
}

/*************************************************************************************************
-----------------------    Get general informations about problem    ----------------------------
*************************************************************************************************/
int  SolverXpress::get_ncols() const{
	int cols(0);
	int status = XPRSgetintattrib(_xprs, XPRS_COLS, &cols);
	zero_status_check(status, "get number of columns");
	return cols;
}

int SolverXpress::get_nrows() const {
	int rows(0);
	int status = XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
	zero_status_check(status, "get number of rows");
	return rows;
}

int SolverXpress::get_nelems() const{
	int elems(0);
	int status = XPRSgetintattrib(_xprs, XPRS_ELEMS, &elems);
	zero_status_check(status, "get number of non zero elements");

	return elems;
}

int SolverXpress::get_n_integer_vars() const{
	int n_int_vars(0);
	int status = XPRSgetintattrib(_xprs, XPRS_MIPENTS, &n_int_vars);
	zero_status_check(status, "get number of integer variables");
	return n_int_vars;
}

void SolverXpress::get_obj(double* obj, int first, int last) const{
	int status = XPRSgetobj(_xprs, obj, first, last);
	zero_status_check(status, "get objective function");
}

void SolverXpress::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, 
                    int first, int last) const{
	// Need to add 1 to indices to avoid objective a index 0
	int status = XPRSgetrows(_xprs, mstart, mclind, dmatval, size, nels, first, last);
	zero_status_check(status, "get rows");
}

void SolverXpress::get_row_type(char* qrtype, int first, int last) const{
	int status = XPRSgetrowtype(_xprs, qrtype, first, last);
	zero_status_check(status, "get rows types");
}

void SolverXpress::get_rhs(double* rhs, int first, int last) const{
	int status = XPRSgetrhs(_xprs, rhs, first, last);
	zero_status_check(status, "get RHS");
}

void SolverXpress::get_rhs_range(double* range, int first, int last) const{
	int status = XPRSgetrhsrange(_xprs, range, first, last);
	zero_status_check(status, "get RHS of range rows");
}

void SolverXpress::get_col_type(char* coltype, int first, int last) const{
	int status = XPRSgetcoltype(_xprs, coltype, first, last);
	zero_status_check(status, "get type of columns");
}

int SolverXpress::get_row_index(std::string const& name) const{
	std::cout << "Seems not working as XPRESS renaims the rows \"Ri\" " << std::endl;
	int id = 0;
	int status = XPRSgetindex(_xprs, 1, name.c_str(), &id);
	return id;
}

int SolverXpress::get_col_index(std::string const& name) const{
	std::cout << "Seems not working as XPRESS renaims the rows \"Ci\" " << std::endl;
	int id = 0;
	int status = XPRSgetindex(_xprs, 2, name.c_str(), &id);
	return id;
}

void SolverXpress::get_lb(double* lb, int first, int last) const{
	int status = XPRSgetlb(_xprs, lb, first, last);
	zero_status_check(status, "get lower bounds of variables");
}

void SolverXpress::get_ub(double* ub, int first, int last) const{
	int status = XPRSgetub(_xprs, ub, first, last);
	zero_status_check(status, "get upper bounds of variables");
}

/*************************************************************************************************
------------------------------    Methods to modify problem    ----------------------------------
*************************************************************************************************/
void SolverXpress::fix_first_stage(std::vector<double> const& x0){
    std::cout << "TO DO IF NEEDED" << std::endl;
    std::exit(0);
}

void SolverXpress::add_cut(std::vector<double> const& s, std::vector<double> const& x0, 
                    double rhs){
	std::cout << "XPRESS add cuts: TO DO IF NEEDED" << std::endl;
    std::exit(0);
}

void SolverXpress::del_rows(int first, int last){
	std::vector<int> mindex(last - first + 1);
	for (int i = 0; i < last - first + 1; i++) {
		mindex[i] = first + i;
	}
	int status = XPRSdelrows(_xprs, last - first + 1, mindex.data());
	zero_status_check(status, "delete rows");
}

void SolverXpress::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
		            const double* range, const int* mstart, const int* mclind, 
                    const double* dmatval){
    int status = XPRSaddrows(_xprs, newrows, newnz, qrtype, rhs, range, mstart, mclind, dmatval);
	zero_status_check(status, "add rows");

}

void SolverXpress::add_cols(int newcol, int newnz, const double* objx, const int* mstart, 
                    const int* mrwind, const double* dmatval, const double* bdl, 
                    const double* bdu){
	int status = XPRSaddcols(_xprs, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu);
	zero_status_check(status, "add columns");
}

void SolverXpress::add_name(int type, const char* cnames, int indice){
	int status = XPRSaddnames(_xprs, type, cnames, indice, indice);
	zero_status_check(status, "add names");
}

void SolverXpress::chg_obj(int nels, const int* mindex, const double* obj){
	int status = XPRSchgobj(_xprs, nels, mindex, obj);
	zero_status_check(status, "change objective");
}

void SolverXpress::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd){
	int status = XPRSchgbounds(_xprs, nbds, mindex, qbtype, bnd);
	zero_status_check(status, "change bounds");
}

void SolverXpress::chg_col_type(int nels, const int* mindex, const char* qctype) const{
	int status = XPRSchgcoltype(_xprs, nels, mindex, qctype);
	zero_status_check(status, "change column types");
}

void SolverXpress::chg_rhs(int id_row, double val){
	int status = XPRSchgrhs(_xprs, 1, std::vector<int>(1,id_row).data(), 
		std::vector<double>(1,val).data() );
	zero_status_check(status, "change rhs");
}

void SolverXpress::chg_coef(int id_row, int id_col, double val){
	int status = XPRSchgcoef(_xprs, id_row, id_col, val);
	zero_status_check(status, "change matrix coefficient");
}
	
/*************************************************************************************************
-----------------------------    Methods to solve the problem    ---------------------------------
*************************************************************************************************/    
void SolverXpress::solve_lp(int& lp_status){
	int status = XPRSlpoptimize(_xprs, "");
	zero_status_check(status, "solve problem as LP");

	int xprs_status(0);
	status = XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &xprs_status);
	zero_status_check(status, "get LP status after LP solve");
	
	if (xprs_status == XPRS_LP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (xprs_status == XPRS_LP_INFEAS) {
		lp_status = INFEASIBLE;
	}
	else if (xprs_status == XPRS_LP_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "Error : UNKNOWN XPRESS STATUS IS : " << xprs_status << std::endl;
	}
}

void SolverXpress::solve_mip(int& lp_status){
	int status(0);
	status = XPRSmipoptimize(_xprs, "");
	zero_status_check(status, "solve problem as MIP");

	int xprs_status(0);
	status = XPRSgetintattrib(_xprs, XPRS_MIPSTATUS, &xprs_status);
	zero_status_check(status, "get MIP status after MIP solve");

	if (xprs_status == XPRS_MIP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (xprs_status == XPRS_MIP_INFEAS) {
		lp_status = INFEASIBLE;
	}
	else if (xprs_status == XPRS_MIP_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "XPRESS STATUS IS : " << xprs_status << std::endl;
	}
}
	
/*************************************************************************************************
-------------------------    Methods to get solutions information    -----------------------------
*************************************************************************************************/
void SolverXpress::get_basis(int* rstatus, int* cstatus) const{
	int status = XPRSgetbasis(_xprs, rstatus, cstatus);
	zero_status_check(status, "get basis");
}

void SolverXpress::get_mip_value(double& val) const{
	int status = XPRSgetdblattrib(_xprs, XPRS_MIPOBJVAL, &val);
	zero_status_check(status, "get MIP value");
}

void SolverXpress::get_lp_value(double& val) const{
	int status = XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &val);
	zero_status_check(status, "get LP value");
}

void SolverXpress::get_simplex_ite(int& result) const{
	int status = XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
	zero_status_check(status, "get simplex iterations");
}

void SolverXpress::get_LP_sol(double* primals, double* slacks, double* duals, 
                    double* reduced_costs){
	int status = XPRSgetlpsol(_xprs, primals, slacks, duals, reduced_costs);
	zero_status_check(status, "get LP sol");
}

void SolverXpress::get_MIP_sol(double* primals, double* slacks){
	int status = XPRSgetmipsol(_xprs, primals, slacks);
	zero_status_check(status, "get MIP sol");
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels    ---------------------------
*************************************************************************************************/
void SolverXpress::set_output_log_level(int loglevel){
	int status = XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
	zero_status_check(status, "set message stream to solver stream");

	if (loglevel == 1 || loglevel == 3) {
		int status = XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
		zero_status_check(status, "set log level");
	}
	else {
		int status = XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
		zero_status_check(status, "set log level");
	}
}

void SolverXpress::set_algorithm(std::string const& algo){
	if (algo == "BARRIER") {
		int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
		zero_status_check(status, "set barrier algorithm");
	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
		zero_status_check(status, "set barrier algorithm");
		status = XPRSsetintcontrol(_xprs, XPRS_CROSSOVER, 0);
		zero_status_check(status, "desactivate barrier crossover");
	}
	else if(algo == "DUAL"){
		int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);
		zero_status_check(status, "set dual simplex algorithm");
	}else{
		std::cout << "Error: invalid algorithm " << algo << std::endl;
		std::exit(0);
	}
}

void SolverXpress::set_threads(int n_threads){
	int status = XPRSsetintcontrol(_xprs, XPRS_THREADS, n_threads);
	zero_status_check(status, "set threads");
}

void SolverXpress::scaling(int scale){
	int status = XPRSsetintcontrol(_xprs, XPRS_SCALING, scale);
	zero_status_check(status, "set scaling level");
}

void SolverXpress::presolve(int presolve){
	int status = XPRSsetintcontrol(_xprs, XPRS_PRESOLVE, presolve);
	zero_status_check(status, "set presolve value");
}

void SolverXpress::optimality_gap(double gap){
	int status = XPRSsetdblcontrol(_xprs, XPRS_OPTIMALITYTOL, gap);
	zero_status_check(status, "set optimality gap");
}

void SolverXpress::set_simplex_iter(int iter){
	int status = XPRSsetdblcontrol(_xprs, XPRS_BARITERLIMIT, iter);
	zero_status_check(status, "set barrier max iter");

	status = XPRSsetdblcontrol(_xprs, XPRS_LPITERLIMIT, iter);
	zero_status_check(status, "set simplex max iter");
}

void SolverXpress::numerical_emphasis(int val){
	//int status = 
	//zero_status_check(status, "set numerical emphasis");
	std::cout << "NUMERICAL EMPHASIS NOT FOUND FOR XPRESS" << std::endl;
}

void XPRS_CC optimizermsg(XPRSprob prob, void* strPtr, const char* sMsg, int nLen,
	int nMsglvl) {
	std::list<std::ostream* >* ptr = NULL;
	if (strPtr != NULL)ptr = (std::list<std::ostream* >*)strPtr;
	switch (nMsglvl) {

		/* Print Optimizer error messages and warnings */
	case 4: /* error */
	case 3: /* warning */
	case 2: /* dialogue */
	case 1: /* information */
		if (ptr != NULL) {
			for (auto const& stream : *ptr)
				* stream << sMsg << std::endl;
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

void errormsg(XPRSprob & _xprs, const char* sSubName, int nLineNo, int nErrCode) {
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