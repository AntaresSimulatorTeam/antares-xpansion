#include "SolverCbc.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor    --------------------------------
*************************************************************************************************/
int SolverCbc::_NumberOfProblems = 0;

SolverCbc::SolverCbc() {
	int status = 0;
	if (_NumberOfProblems == 0) {
		// No environment to instanciate
	}

	_NumberOfProblems += 1;
}

SolverCbc::SolverCbc(const SolverAbstract::Ptr fictif) : SolverCbc() {
	// Try to cast the solver in fictif to a SolverCPLEX
	if (SolverCbc* c = dynamic_cast<SolverCbc*>(fictif.get()))
	{
		_clp_inner_solver = OsiClpSolverInterface(c->_clp_inner_solver);
		_cbc = CbcModel(_clp_inner_solver);
		set_output_log_level(0);
	}
	else {
		_NumberOfProblems -= 1;
		std::cout << "Failed to cast fictif prob into SolverCplex in SolverCplex copy constructor"
			<< std::endl;
		std::exit(0);
	}
}

SolverCbc::~SolverCbc() {
	_NumberOfProblems -= 1;
	free();

	if (_NumberOfProblems == 0) {
		// No environment to close.
	}
}

int SolverCbc::get_number_of_instances()
{
	return _NumberOfProblems;
}

/*************************************************************************************************
-----------------------------------    Output stream management    ------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments    ---------------
*************************************************************************************************/
void SolverCbc::init() {
	_clp_inner_solver = OsiClpSolverInterface();
	_cbc = CbcModel(_clp_inner_solver);
	set_output_log_level(0);
}

void SolverCbc::free() {
}


/*************************************************************************************************
-------------------------------    Reading & Writing problems    -------------------------------
*************************************************************************************************/
void SolverCbc::write_prob(const char* name, const char* flags) const{

	if (std::string(flags) == "LP") {
		_cbc.solver()->writeLpNative(name, NULL, NULL);
	}
	else if (std::string(flags) == "MPS") {

		const int numcols = get_ncols();
		std::shared_ptr<char[]> shared_integrality(new char[numcols]);
		char* integrality = shared_integrality.get();
		CoinCopyN(_cbc.solver()->getColType(false), numcols, integrality);

		bool hasInteger = false;
		for (int i = 0; i < numcols; ++i) {
			if (_cbc.solver()->isInteger(i)) {
				hasInteger = true;
				break;
			}
		}

		CoinMpsIO writer;
		writer.setInfinity(_cbc.solver()->getInfinity());
		writer.passInMessageHandler(_cbc.solver()->messageHandler());

		writer.setMpsData(
			*(_cbc.solver()->getMatrixByCol()),
			_cbc.solver()->getInfinity(),
			_cbc.solver()->getColLower(),
			_cbc.solver()->getColUpper(),
			_cbc.solver()->getObjCoefficients(),
			hasInteger ? integrality : NULL,
			_cbc.solver()->getRowLower(),
			_cbc.solver()->getRowUpper(),
			_cbc.solver()->getColNames(),
			_cbc.solver()->getRowNames()
		);

		std::string probName = "";
		_cbc.solver()->getStrParam(OsiProbName, probName);
		writer.setProblemName(probName.c_str());

		double objOffset = 0.0;
		_cbc.solver()->getDblParam(OsiObjOffset, objOffset);
		writer.setObjectiveOffset(objOffset);

		writer.writeMps(name, 0 /*gzip it*/, 1, 1,
			NULL, 0, NULL);
	}
	else {
		std::cout << "ERROR : Write prob - unknown format type " << flags << std::endl;
		std::exit(1);
	}
}

void SolverCbc::read_prob(const char* prob_name, const char* flags){
	std::string clp_flags = "mps";
	if (std::string(flags) == "LP") {
		clp_flags = "lp";
	}

	int status = _clp_inner_solver.readMps(prob_name, clp_flags.c_str());
	zero_status_check(status, "read problem");

	// Affectation of new Clp interface to Cbc
	// As CbcModel _cbc is modified, need to set log level to 0 again
	_cbc = CbcModel(_clp_inner_solver);	
	set_output_log_level(0);
}

void SolverCbc::copy_prob(const SolverAbstract::Ptr fictif_solv){
	std::cout << "Copy CBC problem : TO DO WHEN NEEDED" << std::endl;
	std::exit(0);
}

/*************************************************************************************************
-----------------------    Get general informations about problem    ----------------------------
*************************************************************************************************/
int  SolverCbc::get_ncols() const{
	int cols(0);
	//cols = _cbc.solver()->getNumCols();
	cols = _cbc.getNumCols();
	return cols;
}

int SolverCbc::get_nrows() const {
	int rows(0);
	rows = _cbc.solver()->getNumRows();
	return rows;
}

int SolverCbc::get_nelems() const{
	int elems(0);
	elems = _cbc.solver()->getNumElements();
	return elems;
}

int SolverCbc::get_n_integer_vars() const{
	int n_int_vars(0);
	n_int_vars = _cbc.solver()->getNumIntegers();
	return n_int_vars;
}

void SolverCbc::get_obj(double* obj, int first, int last) const{
	const int nvars = get_ncols();
	const double* internalObj = _cbc.solver()->getObjCoefficients();

	for (int i = first; i < last + 1; i++) {
		obj[i - first] = internalObj[i];
	}
}

void SolverCbc::get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels,
                    int first, int last) const{
	CoinPackedMatrix matrix = *_cbc.solver()->getMatrixByRow();
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
		mclind[i - firstIndexToReturn] = column[i];
		dmatval[i - firstIndexToReturn] = vals[i];
	}
	*nels = nelemsToReturn;
}

void SolverCbc::get_row_type(char* qrtype, int first, int last) const{
	const double* rowLower = _cbc.solver()->getRowLower();
	const double* rowUpper = _cbc.solver()->getRowUpper();

	std::vector<int> whichBound(get_nrows());
	for (int i(first); i < last + 1; i++) {

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

void SolverCbc::get_rhs(double* rhs, int first, int last) const{
	const double* rowLower = _cbc.solver()->getRowLower();
	const double* rowUpper = _cbc.solver()->getRowUpper();

	for (int i = first; i < last + 1; i++) {

		if (rowLower[i] == rowUpper[i]) {
			rhs[i - first] = rowUpper[i];
		}
		else if (rowLower[i] > -COIN_DBL_MAX) {
			if (rowUpper[i] < COIN_DBL_MAX) {
				std::cout << "ERROR : Row " << i << " has two different RHS, both right and left." << std::endl;
				std::exit(1);
			}
			else {
				rhs[i - first] = rowLower[i];
			}
		}
		else if (rowUpper[i] < COIN_DBL_MAX) {
			rhs[i - first] = rowUpper[i];
		}
		else {
			std::cout << "ERROR : Row " << i << " in unconstrained. No RHS found." << std::endl;
			std::exit(1);
		}
	}
}

void SolverCbc::get_rhs_range(double* range, int first, int last) const{
	std::cout << "ERROR : get rhs range not implemented in the interface for COIN CLP-CBC" << std::endl;
	std::cout << "ERROR : range constraints have to be set as two different constraints." << std::endl;
	std::exit(1);
}

void SolverCbc::get_col_type(char* coltype, int first, int last) const{
	const double* colLower = _cbc.solver()->getColLower();
	const double* colUpper = _cbc.solver()->getColUpper();

	for (int i(first); i < last + 1; i++) {
		if (_cbc.solver()->isInteger(i)) {
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

void SolverCbc::get_lb(double* lb, int first, int last) const{
	const double* colLower = _cbc.solver()->getColLower();

	for (int i(first); i < last + 1; i++) {
		lb[i - first] = colLower[i];
	}
}

void SolverCbc::get_ub(double* ub, int first, int last) const{
	const double* colUpper = _cbc.solver()->getColUpper();

	for (int i(first); i < last + 1; i++) {
		ub[i - first] = colUpper[i];
	}
}

int SolverCbc::get_row_index(std::string const& name) const {
	int id = 0;
	int nrows = get_nrows();
	while (id < nrows) {
		if (_cbc.solver()->getRowName(id) == name) {
			return id;
		}
		id++;
	}
	return -1;
}

int SolverCbc::get_col_index(std::string const& name) const {
	int id = 0;
	int ncols = get_ncols();
	while (id < ncols) {
		if (_cbc.solver()->getColName(id) == name) {
			return id;
		}
		id++;
	}
	return -1;
}

int SolverCbc::get_row_names(int first, int last, std::vector<std::string>& names) const
{
	std::vector<std::string> solver_row_names = _cbc.solver()->getRowNames();
	if (solver_row_names.size() < names.size()) {
		std::cout << "ERROR : all required rows don't have a name. Impossible to get row names."
			<< std::endl;
		std::exit(1);
	}

	for (int i(first); i < last + 1; i++) {
		names[i - first] = solver_row_names[i];
	}
	return 0;
}

int SolverCbc::get_col_names(int first, int last, std::vector<std::string>& names) const
{
	std::vector<std::string> solver_col_names = _cbc.solver()->getColNames();
	if (solver_col_names.size() < names.size()) {
		std::cout << "ERROR : all required columns don't have a name. Impossible to get col names."
			<< std::endl;
		std::exit(1);
	}

	for (int i(first); i < last + 1; i++) {
		names[i - first] = solver_col_names[i];
	}
	return 0;
}

/*************************************************************************************************
------------------------------    Methods to modify problem    ----------------------------------
*************************************************************************************************/

void SolverCbc::del_rows(int first, int last){
	std::vector<int> mindex(last - first + 1);
	for (int i = 0; i < last - first + 1; i++) {
		mindex[i] = first + i;
	}
	_cbc.solver()->deleteRows(last - first + 1, mindex.data());
}

void SolverCbc::add_rows(int newrows, int newnz, const char* qrtype, const double* rhs,
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
	_cbc.solver()->addRows(newrows, mstart, mclind, dmatval, rowLower.data(), rowUpper.data());
}

void SolverCbc::add_cols(int newcol, int newnz, const double* objx, const int* mstart,
                    const int* mrwind, const double* dmatval, const double* bdl, 
                    const double* bdu){

	std::vector<int> colStart(newcol + 1);
	for (int i(0); i < newcol; i++) {
		colStart[i] = mstart[i];
	}
	colStart[newcol] = newnz;

	_cbc.solver()->addCols(newcol, colStart.data(), mrwind, dmatval, bdl, bdu, objx);
}

void SolverCbc::add_name(int type, const char* cnames, int indice){
	std::cout << "ERROR : addnames not implemented in the CLP interface." << std::endl;
	std::exit(1);
}

void SolverCbc::chg_obj(int nels, const int* mindex, const double* obj){
	for (int i(0); i < nels; i++) {
		_cbc.solver()->setObjCoeff(mindex[i], obj[i]);
	}
}

void SolverCbc::chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd){
	for (int i(0); i < nbds; i++) {
		if (qbtype[i] == 'L') {
			_cbc.solver()->setColLower(mindex[i], bnd[i]);
		}
		else if (qbtype[i] == 'U') {
			_cbc.solver()->setColUpper(mindex[i], bnd[i]);
		}
		else if (qbtype[i] == 'B') {
			_cbc.solver()->setColLower(mindex[i], bnd[i]);
			_cbc.solver()->setColUpper(mindex[i], bnd[i]);
		}
		else {
			std::cout << "ERROR : Unknown bound type " << qbtype[i] << " for column " << mindex[i] << std::endl;
		std:exit(1);
		}
	}
}

void SolverCbc::chg_col_type(int nels, const int* mindex, const char* qctype) const{
	std::cout << "ERROR : chg_col_type not implemented for CLP as MIPs are not supported by solver." << std::endl;
	std::exit(1);
}

void SolverCbc::chg_rhs(int id_row, double val){
	const double* rowLower = _cbc.solver()->getRowLower();
	const double* rowUpper = _cbc.solver()->getRowUpper();

	if (rowLower[id_row] <= -COIN_DBL_MAX) {
		if (rowUpper[id_row] >= COIN_DBL_MAX) {
			std::cout << "ERROR : unconstrained constraint " << id_row << " in chg_rhs." << std::endl;
			std::exit(1);
		}
		else {
			_cbc.solver()->setRowUpper(id_row, val);
		}
	}
	else {
		if (rowUpper[id_row] >= COIN_DBL_MAX) {
			_cbc.solver()->setRowLower(id_row, val);
		}
		else {
			std::cout << "ERROR : constraint " << id_row << " has both lower and upper bound in chg_rhs." << std::endl;
			std::cout << "Not implemented in CLP interface yet." << std::endl;
			std::exit(1);
		}
	}
}

void SolverCbc::chg_coef(int id_row, int id_col, double val){

	// Very tricky method by method "modifyCoefficient" of OsiClp does not work
	CoinPackedMatrix matrix = *_cbc.solver()->getMatrixByRow();
	const int* column = matrix.getIndices();
	const int* rowLength = matrix.getVectorLengths();
	const CoinBigIndex* rowStart = matrix.getVectorStarts();
	const double* vals = matrix.getElements();

	matrix.modifyCoefficient(id_row, id_col, val);
	_cbc.solver()->replaceMatrix(matrix);
}

void SolverCbc::chg_row_name(int id_row, std::string & name)
{
	_cbc.solver()->setRowName(id_row, name);
}

void SolverCbc::chg_col_name(int id_col, std::string & name)
{
	_cbc.solver()->setColName(id_col, name);
}
	
/*************************************************************************************************
-----------------------------    Methods to solve the problem    ---------------------------------
*************************************************************************************************/    
void SolverCbc::solve_lp(int& lp_status){

	_cbc.solver()->initialSolve();

	if (_cbc.isInitialSolveProvenOptimal()) {
		lp_status = OPTIMAL;
	}
	else if (_cbc.isInitialSolveProvenPrimalInfeasible()) {
		lp_status = INFEASIBLE;
	}
	else if (_cbc.isInitialSolveProvenDualInfeasible()) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "Error : UNKNOWN CBC STATUS after initial solve." << std::endl;
	}
}

void SolverCbc::solve_mip(int& lp_status){

	std::cout << "Cbc solver write before solve" << std::endl;
	write_prob("master_test.mps", "MPS");
	read_prob("master_test.mps", "MPS");

	_cbc.branchAndBound();
	
	/*std::cout << "*********************************************" << std::endl;
	std::cout << "COUCOU CBC STATUS " << _cbc.status() << std::endl;
	std::cout << "COUCOU CBC STATUS " << _cbc.secondaryStatus() << std::endl;
	std::cout << _cbc.isProvenOptimal() << std::endl;
	std::cout << "INFEAS ? " << _cbc.isProvenInfeasible() << std::endl;
	std::cout << "INFEAS ? " << _cbc.isInitialSolveProvenPrimalInfeasible() << std::endl;
	std::cout << "UNBD   ? " << _cbc.isProvenDualInfeasible() << std::endl;
	std::cout << "UNBD   ? " << _cbc.isInitialSolveProvenDualInfeasible() << std::endl;
	std::cout << "*********************************************" << std::endl;*/
	
	if (_cbc.isProvenOptimal()) {
		if (std::abs(_cbc.solver()->getObjValue()) >= 1e20) {
			lp_status = UNBOUNDED;
		}
		else {
			lp_status = OPTIMAL;
		}
	}
	else if (_cbc.isProvenInfeasible() ||
		_cbc.isInitialSolveProvenPrimalInfeasible()) {
		lp_status = INFEASIBLE;
	}
	else if (_cbc.isProvenDualInfeasible() ||
		_cbc.isInitialSolveProvenDualInfeasible()) {
		lp_status = UNBOUNDED;
	}
	else {
		lp_status = UNKNOWN;
		std::cout << "Error : UNKNOWN CBC STATUS after branch and bound complete search." << std::endl;
	}
}
	
/*************************************************************************************************
-------------------------    Methods to get solutions information    -----------------------------
*************************************************************************************************/
void SolverCbc::get_basis(int* rstatus, int* cstatus) const{
	_cbc.solver()->getBasisStatus(cstatus, rstatus);
}

void SolverCbc::get_mip_value(double& val) const{
	val = _cbc.getObjValue();
}

void SolverCbc::get_lp_value(double& val) const{
	val = _cbc.solver()->getObjValue();
}

void SolverCbc::get_simplex_ite(int& result) const{
	result = _cbc.solver()->getIterationCount();
}

void SolverCbc::get_lp_sol(double* primals, double* duals,
                    double* reduced_costs){
	if (primals != NULL) {
		const double* primalSol = _cbc.solver()->getColSolution();
		for (int i(0); i < get_ncols(); i++) {
			primals[i] = primalSol[i];
		}
	}

	if (duals != NULL) {
		const double* dualSol = _cbc.solver()->getRowPrice();
		for (int i(0); i < get_nrows(); i++) {
			duals[i] = dualSol[i];
		}
	}

	if (reduced_costs != NULL) {
		const double* reducedCostSolution = _cbc.solver()->getReducedCost();
		for (int i(0); i < get_ncols(); i++) {
			reduced_costs[i] = reducedCostSolution[i];
		}
	}	
}

void SolverCbc::get_mip_sol(double* primals){
	if (primals != NULL) {
		const double* primalSol = _cbc.getColSolution();
		for (int i(0); i < get_ncols(); i++) {
			primals[i] = primalSol[i];
		}
	}
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels    ---------------------------
*************************************************************************************************/
void SolverCbc::set_output_log_level(int loglevel){
	_clp_inner_solver.passInMessageHandler(&_message_handler);
	_cbc.passInMessageHandler(&_message_handler);
	if (loglevel == 1 || loglevel == 3) {
		_message_handler.setLogLevel(0, 1);  // Coin messages
		_message_handler.setLogLevel(1, 1);  // Clp messages
		_message_handler.setLogLevel(2, 1);  // Presolve messages
		_message_handler.setLogLevel(3, 1);  // Cgl messages
	}
	else {
		_message_handler.setLogLevel(0, 0);  // Coin messages
		_message_handler.setLogLevel(1, 0);  // Clp messages
		_message_handler.setLogLevel(2, 0);  // Presolve messages
		_message_handler.setLogLevel(3, 0);  // Cgl messages
		_message_handler.setLogLevel(0);
	}
}

void SolverCbc::set_algorithm(std::string const& algo){
	if (algo == "BARRIER") {

	}
	else if (algo == "BARRIER_WO_CROSSOVER") {
		
	}
	else if(algo == "DUAL"){
		
	}else{
		std::cout << "Error: invalid algorithm " << algo << std::endl;
		std::exit(0);
	}

	std::cout << "ERROR : Algortihm handling not implemented in the interface for CBC" << std::endl;
	std::exit(1);
}

void SolverCbc::set_threads(int n_threads){
	_cbc.setNumberThreads(n_threads);
}

void SolverCbc::optimality_gap(double gap){
	std::cout << "ERROR : Optimality gap handling not implemented in the interface for CBC" << std::endl;
	std::exit(1);
}

void SolverCbc::set_simplex_iter(int iter){
	std::cout << "ERROR : Simplex iterations not implemented in the interface for CBC" << std::endl;
	std::exit(1);
}