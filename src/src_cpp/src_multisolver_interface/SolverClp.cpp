#include "SolverClp.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor    --------------------------------
*************************************************************************************************/
int SolverClp::_NumberOfProblems = 0;

SolverClp::SolverClp() {
	int status = 0;
	if (_NumberOfProblems == 0) {
		// No environment to instanciate
	}

	_NumberOfProblems += 1;
	_clp = NULL;
}

SolverClp::SolverClp(const SolverAbstract::Ptr fictif) : SolverClp() {
	// Try to cast the solver in fictif to a SolverCPLEX
	if (SolverClp* c = dynamic_cast<SolverClp*>(fictif.get()))
	{
		_clp = ClpSimplex(c->_clp);
	}
	else {
		_NumberOfProblems -= 1;
		std::cout << "Failed to cast fictif prob into SolverCplex in SolverCplex copy constructor"
			<< std::endl;
		std::exit(0);
	}
}

SolverClp::~SolverClp() {

	_NumberOfProblems -= 1;
	free();

	if (_NumberOfProblems == 0) {
	}
}

int SolverClp::get_number_of_instances()
{
	return _NumberOfProblems;
}

/*************************************************************************************************
-----------------------------------    Output stream management    ------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments    ---------------
*************************************************************************************************/
void SolverClp::init() {
	_clp = ClpSimplex();
	set_output_log_level(0);
}

void SolverClp::free() {
	//_clp = ClpSimplex();
}


/*************************************************************************************************
-------------------------------    Reading & Writing problems    -------------------------------
*************************************************************************************************/
void SolverClp::write_prob(const char* name, const char* flags) const{
	std::string nFlags = "";
	if (std::string(flags) == "LP") {
		nFlags = "-l";
	}
	_clp.writeMps(name, 1);
}

void SolverClp::read_prob(const char* prob_name, const char* flags){
	set_output_log_level(0);
	if (std::string(flags) == "LP") {
		_clp.readLp(prob_name);
	}
	else {
		_clp.readMps(prob_name, true, false);
	}
}

void SolverClp::copy_prob(const SolverAbstract::Ptr fictif_solv){
	std::cout << "Copy Clp problem : TO DO WHEN NEEDED" << std::endl;
	std::exit(0);
}

/*************************************************************************************************
-----------------------    Get general informations about problem    ----------------------------
*************************************************************************************************/
int  SolverClp::get_ncols() const{
	return _clp.getNumCols();
}

int SolverClp::get_nrows() const {
	return _clp.getNumRows();
}

int SolverClp::get_nelems() const{
	return _clp.getNumElements();
}

int SolverClp::get_n_integer_vars() const{
	int nvars = get_ncols();
	int n_int_vars(0);
	for (int i(0); i < nvars; i++) {
		if (_clp.isInteger(i)) {
			n_int_vars += 1;
		}
	}
	return n_int_vars;
}

void SolverClp::get_obj(double* obj, int first, int last) const{
	const int nvars = get_ncols();
	double* internalObj = _clp.objective();

	for (int i = first; i < last + 1; i++) {
		obj[i - first] = internalObj[i];
	}
}

void SolverClp::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels,
                    int first, int last) const{

	CoinPackedMatrix matrix = *_clp.matrix();
	matrix.reverseOrdering();
	const int* column = matrix.getIndices();
	const int* rowLength = matrix.getVectorLengths();
	const CoinBigIndex* rowStart = matrix.getVectorStarts();
	const double* vals = matrix.getElements();
	
	int firstIndexToReturn = rowStart[first];
	int lastIndexToReturn = rowStart[last + 1] - 1;
	int nelemsToReturn = lastIndexToReturn - firstIndexToReturn + 1;
	// Need to take into account the offset of rowstart as _clp.matrix
	// returnes the entire matrix
	for (int i = first; i < last + 2; i++) {
		mstart[i - first] = rowStart[i] - rowStart[first];
	}
	
	for (int i = firstIndexToReturn; i < lastIndexToReturn + 1; i++) {
		mclind[i - firstIndexToReturn]	= column[i];
		dmatval[i - firstIndexToReturn] = vals[i];
	}
	*nels = nelemsToReturn;

}

void SolverClp::get_row_type(char* qrtype, int first, int last) const{
	const double* rowLower = _clp.getRowLower();
	const double* rowUpper = _clp.getRowUpper();

	std::vector<int> whichBound(get_nrows());
	for (int i = first; i < last + 1; i++) {

		if (rowLower[i] == rowUpper[i]) {
			qrtype[i - first] = 'E';
		}
		else if (rowLower[i] > -COIN_DBL_MAX) {
			if (rowUpper[i] < COIN_DBL_MAX) {
				std::cout << "ERROR : Row " << i << " has two different RHS, both right and left." << std::endl;
				std::exit(1);
			}
			else {
				qrtype[i - first] = 'G';
			}
		}
		else if (rowUpper[i] < COIN_DBL_MAX) {
			qrtype[i - first] = 'L';
		}
		else {
			std::cout << "ERROR : Row " << i << " in unconstrained. No RHS found." << std::endl;
			std::exit(1);
		}
	}
}

void SolverClp::get_rhs(double* rhs, int first, int last) const{
	const double* rowLower = _clp.getRowLower();
	const double* rowUpper = _clp.getRowUpper();

	for (int i = first; i < last + 1; i++) {
		if (rowLower[i] == rowUpper[i]) {
			rhs[i - first] = rowLower[i];
		}
		else if (rowLower[i] > -COIN_DBL_MAX) {
			if (rowUpper[i] < COIN_DBL_MAX) {
				std::cout << "ERROR : Row " << i << " has two different RHS, both right and left." << std::endl;
				std::cout << rowLower[i] << " < Row[" << i << "] < " << rowUpper[i] << std::endl;
				std::exit(1);
			}
			else {
				rhs[i - first] = rowLower[i];
			}
		}
		else if (rowUpper[i] < COIN_DBL_MAX) {
			rhs[i - first] = rowUpper[i];
		}
		else{
			std::cout << "ERROR : Row " << i << " in unconstrained. No RHS found." << std::endl;
			std::exit(1);
		}
	}
}

void SolverClp::get_rhs_range(double* range, int first, int last) const{
	std::cout << "ERROR : get rhs range not implemented for COIN CLP interface" << std::endl;
	std::exit(1);
}

void SolverClp::get_col_type(char* coltype, int first, int last) const{
	
	const double* colLower = _clp.getColLower();
	const double* colUpper = _clp.getColUpper();

	for (int i = first; i < last + 1; i++) {
		if (_clp.isInteger(i)) {
			if (colLower[i] == 0 && colUpper[i] == 1) {
				coltype[i - first] = 'B';
			}
			else {
				coltype[i - first] = 'I';
			}
		}
		else {
			coltype[i - first] = 'C';
		}
	}
}

void SolverClp::get_lb(double* lb, int first, int last) const{
	const double* colLower = _clp.getColLower();

	for (int i = first; i < last + 1; i++) {
		lb[i - first] = colLower[i];
	}
}

void SolverClp::get_ub(double* ub, int first, int last) const{
	const double* colUpper = _clp.getColUpper();

	for (int i = first; i < last + 1; i++) {
		ub[i - first] = colUpper[i];
	}
}

int SolverClp::get_row_index(std::string const& name) const {
	int id = 0;
	int nrows = get_nrows();
	while (id < nrows) {
		if (_clp.getRowName(id) == name) {
			return id;
		}
		id++;
	}
	return -1;
}

int SolverClp::get_col_index(std::string const& name) const {
	int id = 0;
	int ncols = get_ncols();
	while (id < ncols) {
		if (_clp.getColumnName(id) == name) {
			return id;
		}
		id++;
	}
	return -1;
}

int SolverClp::get_row_names(int first, int last, std::vector<std::string>& names) const
{
	for (int i = first; i < last + 1; i++) {
		names[i - first] = _clp.getRowName(i);
	}
	return 0;
}

int SolverClp::get_col_names(int first, int last, std::vector<std::string>& names) const
{
	for (int i = first; i < last + 1; i++) {
		names[i - first] = _clp.getColumnName(i);
	}
	return 0;
}

/*************************************************************************************************
------------------------------    Methods to modify problem    ----------------------------------
*************************************************************************************************/

void SolverClp::del_rows(int first, int last){
	std::vector<int> mindex(last - first + 1);
	for (int i = 0; i < last - first + 1; i++) {
		mindex[i] = first + i;
	}
	_clp.deleteRows(last - first + 1, mindex.data());
}

void SolverClp::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
		            const double* range, const int* mstart, const int* mclind, 
                    const double* dmatval){
	
	std::vector<double> rowLower(newrows);
	std::vector<double> rowUpper(newrows);
	for (int i(0); i < newrows; i++) {
		if (qrtype[i] == 'L') {
			rowUpper[i] = rhs[i];
			rowLower[i] = -COIN_DBL_MAX;
		}
		else if (qrtype[i] == 'G') {
			rowUpper[i] = COIN_DBL_MAX;
			rowLower[i] = rhs[i];
		}
		else if (qrtype[i] == 'E') {
			rowUpper[i] = rhs[i];
			rowLower[i] = rhs[i];
		}
		else {
			std::cout << "ERROR : add rows, qrtype " << qrtype[i] << " of row " << i << " to add unknown." << std::endl;
			std::exit(1);
		}
	}

	_clp.addRows(newrows, rowLower.data(), rowUpper.data(), mstart, mclind, dmatval);
}

void SolverClp::add_cols(int newcol, int newnz, const double* objx, const int* mstart,
                    const int* mrwind, const double* dmatval, const double* bdl, 
                    const double* bdu){
	
	std::vector<int> colStart(newcol + 1);
	for (int i(0); i < newcol; i++) {
		colStart[i] = mstart[i];
	}
	colStart[newcol] = newnz;

	_clp.addColumns(newcol, bdl, bdu, objx, colStart.data(), mrwind, dmatval);	
}

void SolverClp::add_name(int type, const char* cnames, int indice){
	std::cout << "ERROR : addnames not implemented in the CLP interface." << std::endl;
	std::exit(1);
}

void SolverClp::chg_obj(int nels, const int* mindex, const double* obj){
	for (int i(0); i < nels; i++) {
		_clp.setObjCoeff(mindex[i], obj[i]);
	}
}

void SolverClp::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd){
	for (int i(0); i < nbds; i++) {
		if (qbtype[i] == 'L') {
			_clp.setColLower(mindex[i], bnd[i]);
		}
		else if (qbtype[i] == 'U') {
			_clp.setColUpper(mindex[i], bnd[i]);
		}
		else if (qbtype[i] == 'B') {
			_clp.setColLower(mindex[i], bnd[i]);
			_clp.setColUpper(mindex[i], bnd[i]);
		}
		else {
			std::cout << "ERROR : Unknown bound type " << qbtype[i] << " for column " << mindex[i] << std::endl;
			std::exit(1);
		}
	}
}

void SolverClp::chg_col_type(int nels, const int* mindex, const char* qctype) const{
	std::cout << "ERROR : chg_col_type not implemented for CLP as MIPs are not supported by solver." << std::endl;
	std::exit(1);
}

void SolverClp::chg_rhs(int id_row, double val){
	const double* rowLower = _clp.getRowLower();
	const double* rowUpper = _clp.getRowUpper();

	if (rowLower[id_row] <= -COIN_DBL_MAX) {
		if (rowUpper[id_row] >= COIN_DBL_MAX) {
			std::cout << "ERROR : unconstrained constraint " << id_row << " in chg_rhs." << std::endl;
			std::exit(1);
		}
		else {
			_clp.setRowUpper(id_row, val);
		}
	}
	else {
		if (rowUpper[id_row] >= COIN_DBL_MAX) {
			_clp.setRowLower(id_row, val);
		}
		else {
			std::cout << "ERROR : constraint " << id_row << " has both different lower and upper bound in chg_rhs." << std::endl;
			std::cout << "Not implemented in CLP interface yet." << std::endl;
			std::exit(1);
		}
	}
}

void SolverClp::chg_coef(int id_row, int id_col, double val){
	
	CoinPackedMatrix* matrix = _clp.matrix();
	matrix->modifyCoefficient(id_row, id_col, val);
}

void SolverClp::chg_row_name(int id_row, std::string & name)
{
	_clp.setRowName(id_row, name);
}

void SolverClp::chg_col_name(int id_col, std::string & name)
{
	_clp.setColumnName(id_col, name);
}
	
/*************************************************************************************************
-----------------------------    Methods to solve the problem    ---------------------------------
*************************************************************************************************/    
void SolverClp::solve_lp(int& lp_status){

	_clp.dual();

	int clp_status = _clp.status();
	if (clp_status == CLP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (clp_status == CLP_PRIMAL_INFEASIBLE) {
		lp_status = INFEASIBLE;
	}
	else if (clp_status == CLP_DUAL_INFEASIBLE) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
	}
}

void SolverClp::solve_mip(int& lp_status){

	_clp.dual();

	int clp_status = _clp.status();
	if (clp_status == CLP_OPTIMAL) {
		lp_status = OPTIMAL;
	}
	else if (clp_status == CLP_PRIMAL_INFEASIBLE) {
		lp_status = INFEASIBLE;
	}
	else if (clp_status == CLP_DUAL_INFEASIBLE) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
	}
}
	
/*************************************************************************************************
-------------------------    Methods to get solutions information    -----------------------------
*************************************************************************************************/
void SolverClp::get_basis(int* rstatus, int* cstatus) const{
	int ncols = get_ncols();
	for (int i = 0; i < ncols; i++) {
		cstatus[i] = _clp.getColumnStatus(i);
	}

	int nrows = get_nrows();
	for (int i = 0; i < nrows; i++) {
		rstatus[i] = _clp.getRowStatus(i);
	}
}

void SolverClp::get_mip_value(double& val) const{
	val = _clp.objectiveValue();
}

void SolverClp::get_lp_value(double& val) const{
	val = _clp.objectiveValue();
}

void SolverClp::get_simplex_ite(int& result) const{
	result = _clp.numberIterations();
}

void SolverClp::get_lp_sol(double* primals, double* duals,
                    double* reduced_costs){
	if (primals != NULL) {
		double* primalSol = _clp.primalColumnSolution();
		for (int i(0); i < get_ncols(); i++) {
			primals[i] = primalSol[i];
		}
	}
	
	if (duals != NULL) {
		double* dualSol = _clp.dualRowSolution();
		for (int i(0); i < get_nrows(); i++) {
			duals[i] = dualSol[i];
		}
	}

	if (reduced_costs != NULL) {
		double* reducedCostSolution = _clp.dualColumnSolution();
		for (int i(0); i < get_ncols(); i++) {
			reduced_costs[i] = reducedCostSolution[i];
			//std::cout << "RD    " << i << " " << reducedCostSolution[i] << std::endl;
		}
	}
}

void SolverClp::get_mip_sol(double* primals){
	if (primals != NULL) {
		double* primalSol = _clp.primalColumnSolution();
		for (int i(0); i < get_ncols(); i++) {
			primals[i] = primalSol[i];
		}
	}
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels    ---------------------------
*************************************************************************************************/
void SolverClp::set_output_log_level(int loglevel){
	_clp.passInMessageHandler(&_message_handler);
	if (loglevel == 1 || loglevel == 3) {
		_message_handler.setLogLevel(1);
	}
	else {
		_message_handler.setLogLevel(0);
	}
}

void SolverClp::set_algorithm(std::string const& algo){
	if (algo == "BARRIER") {
		std::cout << "ERROR : Barrier algorithm non pris en charge par Clp Simplex." << std::endl;
		std::exit(1);
	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		std::cout << "ERROR : Barrier algorithm non pris en charge par Clp Simplex." << std::endl;
		std::exit(1);
	}
	else if(algo == "DUAL"){
		_clp.setAlgorithm(1);
	}else{
		std::cout << "Error: invalid algorithm " << algo << std::endl;
		std::exit(0);
	}
}

void SolverClp::set_threads(int n_threads){
	_clp.setNumberThreads(n_threads);
}

void SolverClp::optimality_gap(double gap){
	std::cout << "ERROR : Optimality gap handling not implemented with Clp" << std::endl;
	std::exit(1);
}

void SolverClp::set_simplex_iter(int iter){
	_clp.setMaximumIterations(iter);
}