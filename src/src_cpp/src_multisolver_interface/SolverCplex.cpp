#include "SolverCplex.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor    --------------------------------
*************************************************************************************************/
int SolverCplex::_NumberOfProblems = 0;

SolverCplex::SolverCplex() {
	int status(0);

	// Openning CPLEX environment
	_env = CPXopenCPLEX(&status);
	zero_status_check(status, "open CPLEX");

	_prb = NULL;

	_NumberOfProblems += 1;
}

SolverCplex::SolverCplex(const std::string& name) {
    int status(0);

    // Openning CPLEX environment
    _env = CPXopenCPLEX(&status);
	zero_status_check(status, "open CPLEX");

    // Creating empty problem
    _prb = CPXcreateprob(_env, &status, name.c_str());
    zero_status_check(status, "create problem");

	_NumberOfProblems += 1;
}

SolverCplex::SolverCplex(const SolverAbstract::Ptr fictif) {
    int status(0);

    // Openning CPLEX environment
	_env = CPXopenCPLEX(&status);
    zero_status_check(status, "open CPLEX");

    // Try to cast the solver in fictif to a SolverCPLEX
	if (SolverCplex* c = dynamic_cast<SolverCplex*>(fictif.get()))
	{
		_prb = CPXcloneprob(_env, c->_prb, &status);
        zero_status_check(status, "create problem");

		_NumberOfProblems += 1;
	}
	else {
		std::cout << "Failed to cast fictif prob into SolverCplex in SolverCplex copy constructor" 
            << std::endl;
		std::exit(0);
	}
}

SolverCplex::~SolverCplex() {
	_NumberOfProblems -= 1;
	free();
	
	if (_NumberOfProblems == 0) {
		int status = CPXcloseCPLEX(&_env);
        zero_status_check(status, "close CPLEX environment");
	}
}

int SolverCplex::get_number_of_instances()
{
	return _NumberOfProblems;
}

/*************************************************************************************************
-----------------------------------    Output stream management    ------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments    ---------------
*************************************************************************************************/
void SolverCplex::init() {
	// Creating empty problem
	int status = 0;
	const std::string name = "DefaultProblem_" + std::to_string(_NumberOfProblems);
	_prb = CPXcreateprob(_env, &status, name.c_str());
	zero_status_check(status, "create problem");
}




void SolverCplex::free() {
    int status(0);
    status = CPXfreeprob(_env, &_prb);
    zero_status_check(status, "free problem in memory");
}


/*************************************************************************************************
-------------------------------    Reading & Writing problems    -------------------------------
*************************************************************************************************/
void SolverCplex::write_prob(const char* name, const char* flags) const{
    CPXwriteprob(_env, _prb, name, flags);
}

void SolverCplex::read_prob(const char* prob_name, const char* flags){
	int status = CPXreadcopyprob(_env, _prb, prob_name, flags);
    zero_status_check(status, "read problem");
}

void SolverCplex::copy_prob(const SolverAbstract::Ptr fictif_solv){

}

/*************************************************************************************************
-----------------------    Get general informations about problem    ----------------------------
*************************************************************************************************/
int  SolverCplex::get_ncols() const{
    int cols(0);
	cols = CPXgetnumcols(_env, _prb);
	return cols;
}

int  SolverCplex::get_nrows() const {
    int rows(0);
    rows = CPXgetnumrows(_env, _prb);
    return rows;
}

int  SolverCplex::get_nelems() const{
    int elems = CPXgetnumnz(_env, _prb);
	return elems;
}

int SolverCplex::get_n_integer_vars() const{
	int n_int_vars = CPXgetnumint(_env, _prb);
	int n_bin_vars = CPXgetnumbin(_env, _prb);
	return n_int_vars + n_bin_vars;
}

void SolverCplex::get_obj(double* obj, int first, int last) const{
    int status = CPXgetobj(_env, _prb, obj, first, last);
    zero_status_check(status, "get obj");
}

void SolverCplex::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, 
                    int first, int last) const{
    std::vector<int> surplus(1);
	int status = CPXgetrows(_env, _prb, nels, mstart, mclind, dmatval, size, surplus.data(), first, last);
    zero_status_check(status, "get rows");

	// As other solvers adds the last element in mstart
	mstart[last - first + 1] = size;
}

void SolverCplex::get_row_type(char* qrtype, int first, int last) const{
    int status = CPXgetsense(_env, _prb, qrtype, first, last);
    zero_status_check(status, "get row type");
}

void SolverCplex::get_rhs(double* rhs, int first, int last) const{
    int status = CPXgetrhs(_env, _prb, rhs, first, last);
    zero_status_check(status, "get rhs");
}

void SolverCplex::get_rhs_range(double* range, int first, int last) const{
    int status = CPXgetrngval(_env, _prb, range, first, last);
    zero_status_check(status, "get rhs range");
}

void SolverCplex::get_col_type(char* coltype, int first, int last) const{
    // Declaration en MIP pour creer les types de variables dans la memoire (CPLEX)
    CPXchgprobtype(_env, _prb, CPXPROB_MILP);

    int status = CPXgetctype(_env, _prb, coltype, first, last);
    zero_status_check(status, "get col type");
}

void SolverCplex::get_lb(double* lb, int first, int last) const{
    int status = CPXgetlb(_env, _prb, lb, first, last);
    zero_status_check(status, "get lb");
}

void SolverCplex::get_ub(double* ub, int first, int last) const{
    int status = CPXgetub(_env, _prb, ub, first, last);
    zero_status_check(status, "get ub");
}

int SolverCplex::get_row_index(std::string const& name) const {
	int id;
	int status = CPXgetrowindex(_env, _prb, name.c_str(), &id);
	zero_status_check(status, "get row index");
	return id;
}

int SolverCplex::get_col_index(std::string const& name) const {
	int id;
	int status = CPXgetcolindex(_env, _prb, name.c_str(), &id);
	zero_status_check(status, "get col index");
	return id;
}

int SolverCplex::get_row_names(int first, int last, std::vector<std::string>& names) const
{
	int status = 0;
	const int charSize = 100;
	char cur_name[charSize];
	std::vector<char*> namesPtr(charSize);
	for (int i = 0; i < last - first + 1; i++) {
		status = CPXgetrowname(_env, _prb, namesPtr.data(), cur_name, charSize, 
			NULL, i + first, i + first);
		zero_status_check(status, "get row name");
		names[i] = cur_name;
		memset(cur_name, 0, 100);
	}

	return status;
}

int SolverCplex::get_col_names(int first, int last, std::vector<std::string>& names) const
{
	int status = 0;
	const int charSize = 100;
	char cur_name[charSize];
	std::vector<char*> namesPtr(charSize);
	for (int i = 0; i < last - first + 1; i++) {
		status = CPXgetcolname(_env, _prb, namesPtr.data(), cur_name, charSize, 
			NULL, i + first, i + first);
		zero_status_check(status, "get column name");
		names[i] = cur_name;
		memset(cur_name, 0, 100);
	}

	return status;
}

/*************************************************************************************************
------------------------------    Methods to modify problem    ----------------------------------
*************************************************************************************************/

void SolverCplex::del_rows(int first, int last){
    int status = CPXdelrows(_env, _prb, first, last);
    zero_status_check(status, "delete rows");
}

void SolverCplex::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
		            const double* range, const int* mstart, const int* mclind, 
                    const double* dmatval){
    int status = CPXaddrows(_env, _prb, 0, newrows, newnz, rhs, qrtype, mstart, 
                    mclind, dmatval,NULL, NULL);
    zero_status_check(status, "add rows");
}

void SolverCplex::add_cols(int newcol, int newnz, const double* objx, const int* mstart, 
                    const int* mrwind, const double* dmatval, const double* bdl, 
                    const double* bdu){
    int status = CPXaddcols(_env, _prb, newcol, newnz, objx, mstart, mrwind, 
                    dmatval, bdl, bdu, NULL);
    zero_status_check(status, "add cols");
}

void SolverCplex::add_name(int type, const char* cnames, int indice){
    if (type == 1) {
        // ROW
		type = 'r';
	}
	else if (type == 2) {
        // COLUMN
		type = 'c';
	}
	else {
		std::cout << "ERROR : wrong type sent to add_name" << std::endl;
		std::exit(0);
	}
	int status = CPXchgname(_env, _prb, type, indice, cnames);
    zero_status_check(status, "add name");
}

void SolverCplex::chg_obj(int nels, const int* mindex, const double* obj){
    int status = CPXchgobj(_env, _prb, nels, mindex, obj);
    zero_status_check(status, "change obj");
}

void SolverCplex::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd){
    int status = CPXchgbds(_env, _prb, nbds, mindex, qbtype, bnd);
    zero_status_check(status, "change bounds");
}

void SolverCplex::chg_col_type(int nels, const int* mindex, const char* qctype) const{
    int status = CPXchgctype(_env, _prb, nels, mindex, qctype);
    zero_status_check(status, "change col type");
	if (get_n_integer_vars() == 0) {
		int status = CPXchgprobtype(_env, _prb, CPXPROB_LP);
        zero_status_check(status, "change prob type after change col type");
	}
}

void SolverCplex::chg_rhs(int id_row, double val){
    int ids[1];
	ids[0] = id_row;
	double vals[1];
	vals[0] = val;
	int status = CPXchgrhs(_env, _prb, 1, ids, vals);
    zero_status_check(status, "change rhs");
}

void SolverCplex::chg_coef(int id_row, int id_col, double val){
    int status = CPXchgcoef(_env, _prb, id_row, id_col, val);
    zero_status_check(status, "change coef");
}

void SolverCplex::chg_row_name(int id_row, std::string & name)
{
	const std::vector<int> indices(1, id_row);
	
	std::vector<char*> nameVec(1);
	char nameToChar[100];
	memset(nameToChar, 0, sizeof(nameToChar));
	for (int i(0); i < name.size(); i++) {
		nameToChar[i] = name[i];
	}
	nameVec[0] = nameToChar;

	int status = CPXchgrowname(_env, _prb, 1, indices.data(), nameVec.data());
	zero_status_check(status, "Set row name");
}

void SolverCplex::chg_col_name(int id_col, std::string & name)
{
	const std::vector<int> indices(1, id_col);

	std::vector<char*> nameVec(1);
	char nameToChar[100];
	memset(nameToChar, 0, sizeof(nameToChar));
	for (int i(0); i < name.size(); i++) {
		nameToChar[i] = name[i];
	}
	nameVec[0] = nameToChar;
	int status = CPXchgcolname(_env, _prb, 1, indices.data(), nameVec.data());
	zero_status_check(status, "Set col name");
}
	
/*************************************************************************************************
-----------------------------    Methods to solve the problem    ---------------------------------
*************************************************************************************************/    
void SolverCplex::solve_lp(int& lp_status){
    
    int status = CPXlpopt(_env, _prb);
	zero_status_check(status, "solve prb as lp");

	int cpx_status(0);
	cpx_status = CPXgetstat(_env, _prb);

	if (cpx_status == CPX_STAT_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (cpx_status == CPX_STAT_INFEASIBLE) {
		lp_status = INFEASIBLE;
	}
	else if (cpx_status == CPX_STAT_UNBOUNDED) {
		lp_status = UNBOUNDED;
	}
	else if (cpx_status == CPX_STAT_INForUNBD) {
		lp_status = INForUNBOUND;
	}
	else if(cpx_status == CPX_STAT_OPTIMAL_INFEAS) {
        std::cout << "WARNING: Code CPX_STAT_OPTIMAL_INFEAS treated as optimal" << std::endl;
		lp_status = OPTIMAL;
	}
	else if (cpx_status == CPX_STAT_ABORT_IT_LIM) {
		lp_status = OPTIMAL;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "UNKNOWN CPLEX STATUS: " << cpx_status << std::endl;
        std::exit(0);
	}
}

void SolverCplex::solve_mip(int& lp_status){
    
    if (get_n_integer_vars() == 0) {
		solve_lp(lp_status);
	}
	else {
		int status = CPXmipopt(_env, _prb);
        zero_status_check(status, "solve prb as MIP");

		int cpx_status(0);
		cpx_status = CPXgetstat(_env, _prb);
		if (cpx_status == CPXMIP_OPTIMAL) {
			lp_status = OPTIMAL;
		}
		else if (cpx_status == CPXMIP_INFEASIBLE) {
			lp_status = INFEASIBLE;
		}
		else if (cpx_status == CPXMIP_UNBOUNDED) {
			lp_status = UNBOUNDED;
		}
		else if (cpx_status == CPXMIP_INForUNBD) {
			lp_status = INForUNBOUND;
		}
		else if (cpx_status == CPXMIP_OPTIMAL_TOL) {
			lp_status = OPTIMAL;
		}
		else {
			lp_status = UNKNOWN;
		    std::cout << "UNKNOWN CPLEX STATUS: " << cpx_status << std::endl;
            std::exit(0);
		}
	}
}
	
/*************************************************************************************************
-------------------------    Methods to get solutions information    -----------------------------
*************************************************************************************************/
void SolverCplex::get_basis(int* rstatus, int* cstatus) const{
	CPXgetbase(_env, _prb, cstatus, rstatus);
}

void SolverCplex::get_mip_value(double& val) const{
	CPXgetobjval(_env, _prb, &val);
}

void SolverCplex::get_lp_value(double& val) const{
	CPXgetobjval(_env, _prb, &val);
}

void SolverCplex::get_simplex_ite(int& result) const{
	result = CPXgetitcnt(_env, _prb);
}

void SolverCplex::get_lp_sol(double* primals, double* duals, 
                    double* reduced_costs){
	CPXsolution(_env, _prb, NULL, NULL, primals, duals, NULL, reduced_costs);
}

void SolverCplex::get_mip_sol(double* primals){
	CPXsolution(_env, _prb, NULL, NULL, primals, NULL, NULL, NULL);
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels    ---------------------------
*************************************************************************************************/
void SolverCplex::set_output_log_level(int loglevel){
	if (loglevel == 1 || loglevel == 3) {
		int status = CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_ON);
		zero_status_check(status, "set solver log level");
	}
	else {
		int status = CPXsetintparam(_env, CPXPARAM_ScreenOutput, CPX_OFF);
		zero_status_check(status, "set solver log level");
	}
}

void SolverCplex::set_algorithm(std::string const& algo){
	int status(0);
	if (algo == "BARRIER") {
		status = CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
		zero_status_check(status, "set barrier algorithm");
	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		status = CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_BARRIER);
		zero_status_check(status, "set barrier algorithm");
		status = CPXsetintparam(_env, CPXPARAM_Barrier_Crossover, CPX_ALG_NONE);
		zero_status_check(status, "set crossover param to 0 in barrier");
	}
	else if (algo == "DUAL") {
		status = CPXsetintparam(_env, CPXPARAM_LPMethod, CPX_ALG_DUAL);
		zero_status_check(status, "set dual simplex algorithm");
	}
	else {
		std::cout << "Error: invalid algorithm " << algo << std::endl;
		std::exit(0);
	}
}

void SolverCplex::set_threads(int n_threads){
	int status = CPXsetintparam(_env, CPXPARAM_Threads, n_threads);
	zero_status_check(status, "set threads number");
}

void SolverCplex::optimality_gap(double gap){
	int status = CPXsetdblparam(_env, CPXPARAM_Simplex_Tolerances_Optimality, gap);
	zero_status_check(status, "set optimality tol for simplex");
	
	status = CPXsetdblparam(_env, CPXPARAM_Barrier_ConvergeTol, gap);
	zero_status_check(status, "set optimality gap for barrier");
}

void SolverCplex::set_simplex_iter(int iter){
	int status = CPXsetintparam(_env, CPXPARAM_Simplex_Limits_Iterations, iter);
	zero_status_check(status, "set maximum number of simplex iterations");
}