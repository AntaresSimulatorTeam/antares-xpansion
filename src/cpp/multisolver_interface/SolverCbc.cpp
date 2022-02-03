#include "SolverCbc.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor
--------------------------------
*************************************************************************************************/
int SolverCbc::_NumberOfProblems = 0;

SolverCbc::SolverCbc() {
  int status = 0;
  if (_NumberOfProblems == 0) {
    // No environment to instanciate
  }

  _NumberOfProblems += 1;
  _current_log_level = 0;
}

SolverCbc::SolverCbc(const SolverAbstract::Ptr fictif) : SolverCbc() {
  // Try to cast the solver in fictif to a SolverCPLEX
  if (SolverCbc *c = dynamic_cast<SolverCbc *>(fictif.get())) {
    _clp_inner_solver = OsiClpSolverInterface(c->_clp_inner_solver);
    defineCbcModelFromInnerSolver();
  } else {
    _NumberOfProblems -= 1;
    throw InvalidSolverForCopyException(fictif->get_solver_name(),
                                        get_solver_name());
  }
}

SolverCbc::~SolverCbc() {
  _NumberOfProblems -= 1;
  free();

  if (_NumberOfProblems == 0) {
    // No environment to close.
  }
}

int SolverCbc::get_number_of_instances() { return _NumberOfProblems; }

void SolverCbc::defineCbcModelFromInnerSolver() {
  // Affectation of new Clp interface to Cbc
  // As CbcModel _cbc is modified, need to set log level to 0 again
  _cbc = CbcModel(_clp_inner_solver);
  set_output_log_level(_current_log_level);
}

/*************************************************************************************************
-----------------------------------    Output stream management
------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments
---------------
*************************************************************************************************/
void SolverCbc::init() {
  _clp_inner_solver = OsiClpSolverInterface();
  defineCbcModelFromInnerSolver();
}

void SolverCbc::free() {}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/
void SolverCbc::write_prob_mps(const std::string &filename) {

  const int numcols = get_ncols();
  std::shared_ptr<char[]> shared_integrality(new char[numcols]);
  char *integrality = shared_integrality.get();
  CoinCopyN(_clp_inner_solver.getColType(false), numcols, integrality);

  bool hasInteger = false;
  for (int i = 0; i < numcols; ++i) {
    if (_clp_inner_solver.isInteger(i)) {
      hasInteger = true;
      break;
    }
  }

  CoinMpsIO writer;
  writer.setInfinity(_clp_inner_solver.getInfinity());
  writer.passInMessageHandler(_clp_inner_solver.messageHandler());

  // If the user added cuts or rows but did not added names to them
  // the number of names returned by solver might be different from the
  // actual number of names, resulting in a crash
  std::vector<std::string> rowNames(get_nrows());
  for (int i = 0; i < get_nrows(); ++i) {
    std::string const &name(_clp_inner_solver.getRowName(i));
    if (name == "") {
      std::stringstream buffer;
      buffer << "R" << i;
      rowNames[i] = buffer.str();
    } else {
      rowNames[i] = name;
    }
  }

  std::vector<std::string> colNames(get_ncols());
  for (int i = 0; i < get_ncols(); ++i) {
    std::string const &name(_clp_inner_solver.getColName(i));
    if (name == "") {
      std::stringstream buffer;
      buffer << "C" << i;
      colNames[i] = buffer.str();
    } else {
      colNames[i] = name;
    }
  }

  writer.setMpsData(
      *(_clp_inner_solver.getMatrixByCol()), _clp_inner_solver.getInfinity(),
      _clp_inner_solver.getColLower(), _clp_inner_solver.getColUpper(),
      _clp_inner_solver.getObjCoefficients(), hasInteger ? integrality : NULL,
      _clp_inner_solver.getRowLower(), _clp_inner_solver.getRowUpper(),
      colNames, rowNames);

  std::string probName = "";
  _clp_inner_solver.getStrParam(OsiProbName, probName);
  writer.setProblemName(probName.c_str());

  double objOffset = 0.0;
  _clp_inner_solver.getDblParam(OsiObjOffset, objOffset);
  writer.setObjectiveOffset(objOffset);

  writer.writeMps(filename.c_str(), 0 /*gzip it*/, 1, 1, NULL, 0, NULL);
}

void SolverCbc::write_prob_lp(const std::string &name) {
  _clp_inner_solver.writeLpNative(name.c_str(), NULL, NULL);
}

void SolverCbc::read_prob_mps(const std::string &prob_name) {
  int status = _clp_inner_solver.readMps(prob_name.c_str());
  zero_status_check(status, "read problem");
  defineCbcModelFromInnerSolver();
}

void SolverCbc::read_prob_lp(const std::string &prob_name) {
  int status = _clp_inner_solver.readLp(prob_name.c_str());
  zero_status_check(status, "read problem");
  defineCbcModelFromInnerSolver();
}

void SolverCbc::copy_prob(const SolverAbstract::Ptr fictif_solv) {
  std::string error = "Copy CBC problem : TO DO WHEN NEEDED";
  throw NotImplementedFeatureSolverException(error);
}

/*************************************************************************************************
-----------------------    Get general informations about problem
----------------------------
*************************************************************************************************/
int SolverCbc::get_ncols() const {
  int cols = _clp_inner_solver.getNumCols();
  return cols;
}

int SolverCbc::get_nrows() const {
  int rows = _clp_inner_solver.getNumRows();
  return rows;
}

int SolverCbc::get_nelems() const {
  int elems = _clp_inner_solver.getNumElements();
  return elems;
}

int SolverCbc::get_n_integer_vars() const {
  int n_int_vars = _clp_inner_solver.getNumIntegers();
  return n_int_vars;
}

void SolverCbc::get_obj(double *obj, int first, int last) const {
  const int nvars = get_ncols();
  const double *internalObj = _clp_inner_solver.getObjCoefficients();

  for (int i = first; i < last + 1; i++) {
    obj[i - first] = internalObj[i];
  }
}

void SolverCbc::get_rows(int *mstart, int *mclind, double *dmatval, int size,
                         int *nels, int first, int last) const {
  CoinPackedMatrix matrix = *_clp_inner_solver.getMatrixByRow();
  const int *column = matrix.getIndices();
  const int *rowLength = matrix.getVectorLengths();
  const CoinBigIndex *rowStart = matrix.getVectorStarts();
  const double *vals = matrix.getElements();

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

void SolverCbc::get_row_type(char *qrtype, int first, int last) const {
  const double *rowLower = _clp_inner_solver.getRowLower();
  const double *rowUpper = _clp_inner_solver.getRowUpper();

  std::vector<int> whichBound(get_nrows());
  for (int i(first); i < last + 1; i++) {

    if (rowLower[i] == rowUpper[i]) {
      qrtype[i - first] = 'E';
    } else if (rowLower[i] > -COIN_DBL_MAX) {
      if (rowUpper[i] < COIN_DBL_MAX) {
        std::string error = "ERROR : Row " + std::to_string(i) +
                            " has two different RHS, both right and left.";
        throw GenericSolverException(error);
      } else {
        qrtype[i - first] = 'G';
      }
    } else if (rowUpper[i] < COIN_DBL_MAX) {
      qrtype[i - first] = 'L';
    } else {
      std::string error = "ERROR : Row " + std::to_string(i) +
                          " in unconstrained. No RHS found.";
      throw GenericSolverException(error);
    }
  }
}

void SolverCbc::get_rhs(double *rhs, int first, int last) const {
  const double *rowLower = _clp_inner_solver.getRowLower();
  const double *rowUpper = _clp_inner_solver.getRowUpper();

  for (int i = first; i < last + 1; i++) {

    if (rowLower[i] == rowUpper[i]) {
      rhs[i - first] = rowUpper[i];
    } else if (rowLower[i] > -COIN_DBL_MAX) {
      if (rowUpper[i] < COIN_DBL_MAX) {
        std::string error = "ERROR : Row " + std::to_string(i) +
                            " has two different RHS, both right and left.";
        throw GenericSolverException(error);
      } else {
        rhs[i - first] = rowLower[i];
      }
    } else if (rowUpper[i] < COIN_DBL_MAX) {
      rhs[i - first] = rowUpper[i];
    } else {
      std::string error = "ERROR : Row " + std::to_string(i) +
                          " in unconstrained. No RHS found.";
      throw GenericSolverException(error);
    }
  }
}

void SolverCbc::get_rhs_range(double *range, int first, int last) const {
  std::stringstream buffer;
  buffer << "ERROR : get rhs range not implemented in the interface for COIN "
            "CLP-CBC"
         << std::endl;
  buffer << "ERROR : range constraints have to be set as two different "
            "constraints.";
  throw NotImplementedFeatureSolverException(buffer.str());
}

void SolverCbc::get_col_type(char *coltype, int first, int last) const {
  const double *colLower = _clp_inner_solver.getColLower();
  const double *colUpper = _clp_inner_solver.getColUpper();

  for (int i(first); i < last + 1; i++) {
    if (_clp_inner_solver.isInteger(i)) {
      if (colLower[i] == 0 && colUpper[i] == 1) {
        coltype[i - first] = 'B';
      } else {
        coltype[i - first] = 'I';
      }
    } else {
      coltype[i - first] = 'C';
    }
  }
}

void SolverCbc::get_lb(double *lb, int first, int last) const {
  const double *colLower = _clp_inner_solver.getColLower();

  for (int i(first); i < last + 1; i++) {
    lb[i - first] = colLower[i];
  }
}

void SolverCbc::get_ub(double *ub, int first, int last) const {
  const double *colUpper = _clp_inner_solver.getColUpper();

  for (int i(first); i < last + 1; i++) {
    ub[i - first] = colUpper[i];
  }
}

int SolverCbc::get_row_index(std::string const &name) const {
  int id = 0;
  int nrows = get_nrows();
  while (id < nrows) {
    if (_clp_inner_solver.getRowName(id) == name) {
      return id;
    }
    id++;
  }
  return -1;
}

int SolverCbc::get_col_index(std::string const &name) const {
  int id = 0;
  int ncols = get_ncols();
  while (id < ncols) {
    if (_clp_inner_solver.getColName(id) == name) {
      return id;
    }
    id++;
  }
  return -1;
}

std::vector<std::string> SolverCbc::get_row_names(int first, int last) {
  int size = 1 + last - first;
  std::vector<std::string> names;
  names.reserve(size);

  std::vector<std::string> solver_row_names = _clp_inner_solver.getRowNames();
  if (solver_row_names.size() < size) {
    throw InvalidRowSizeException(size, solver_row_names.size());
  }

  for (int i(first); i < last + 1; i++) {
    names.push_back(solver_row_names[i]);
  }
  return names;
}

std::vector<std::string> SolverCbc::get_col_names(int first, int last) {
  int size = 1 + last - first;
  std::vector<std::string> names;
  names.reserve(size);

  std::vector<std::string> solver_col_names = _clp_inner_solver.getColNames();
  if (solver_col_names.size() < size) {
    throw InvalidColSizeException(size, solver_col_names.size());
  }

  for (int i(first); i < last + 1; i++) {
    names.push_back(solver_col_names[i]);
  }
  return names;
}

/*************************************************************************************************
------------------------------    Methods to modify problem
----------------------------------
*************************************************************************************************/

void SolverCbc::del_rows(int first, int last) {
  std::vector<int> mindex(last - first + 1);
  for (int i = 0; i < last - first + 1; i++) {
    mindex[i] = first + i;
  }
  _clp_inner_solver.deleteRows(last - first + 1, mindex.data());
}

void SolverCbc::add_rows(int newrows, int newnz, const char *qrtype,
                         const double *rhs, const double *range,
                         const int *mstart, const int *mclind,
                         const double *dmatval) {

  std::vector<double> rowLower(newrows);
  std::vector<double> rowUpper(newrows);
  for (int i(0); i < newrows; i++) {
    if (qrtype[i] == 'L') {
      rowUpper[i] = rhs[i];
      rowLower[i] = -COIN_DBL_MAX;
    } else if (qrtype[i] == 'G') {
      rowUpper[i] = COIN_DBL_MAX;
      rowLower[i] = rhs[i];
    } else if (qrtype[i] == 'E') {
      rowUpper[i] = rhs[i];
      rowLower[i] = rhs[i];
    } else {
      std::stringstream buffer;
      buffer << "ERROR : add rows, qrtype " << qrtype[i] << " of row " << i
             << " to add unknown.";
      throw GenericSolverException(buffer.str());
    }
  }
  _clp_inner_solver.addRows(newrows, mstart, mclind, dmatval, rowLower.data(),
                            rowUpper.data());
}

void SolverCbc::add_cols(int newcol, int newnz, const double *objx,
                         const int *mstart, const int *mrwind,
                         const double *dmatval, const double *bdl,
                         const double *bdu) {

  std::vector<int> colStart(newcol + 1);
  for (int i(0); i < newcol; i++) {
    colStart[i] = mstart[i];
  }
  colStart[newcol] = newnz;

  _clp_inner_solver.addCols(newcol, colStart.data(), mrwind, dmatval, bdl, bdu,
                            objx);
}

void SolverCbc::add_name(int type, const char *cnames, int indice) {
  std::string error = "ERROR : addnames not implemented in the CLP interface.";
  throw NotImplementedFeatureSolverException(error);
}

void SolverCbc::chg_obj(const std::vector<int> &mindex,
                        const std::vector<double> &obj) {
  assert(obj.size() == mindex.size());
  for (int i(0); i < obj.size(); i++) {
    _clp_inner_solver.setObjCoeff(mindex[i], obj[i]);
  }
}

void SolverCbc::chg_bounds(const std::vector<int> &mindex,
                           const std::vector<char> &qbtype,
                           const std::vector<double> &bnd) {
  assert(qbtype.size() == mindex.size());
  assert(bnd.size() == mindex.size());
  for (int i(0); i < mindex.size(); i++) {
    if (qbtype[i] == 'L') {
      _clp_inner_solver.setColLower(mindex[i], bnd[i]);
    } else if (qbtype[i] == 'U') {
      _clp_inner_solver.setColUpper(mindex[i], bnd[i]);
    } else if (qbtype[i] == 'B') {
      _clp_inner_solver.setColLower(mindex[i], bnd[i]);
      _clp_inner_solver.setColUpper(mindex[i], bnd[i]);
    } else {
      throw InvalidBoundTypeException(qbtype[i]);
    }
  }
}

void SolverCbc::chg_col_type(const std::vector<int> &mindex,
                             const std::vector<char> &qctype) {
  assert(qctype.size() == mindex.size());
  std::vector<int> bnd_index(1, 0);
  std::vector<char> bnd_type(1, 'U');
  std::vector<double> bnd_val(1, 1.0);

  for (int i = 0; i < mindex.size(); i++) {
    if (qctype[i] == 'C') {
      _clp_inner_solver.setContinuous(mindex[i]);
    } else if (qctype[i] == 'B') {
      _clp_inner_solver.setInteger(mindex[i]);
      bnd_index[0] = mindex[i];
      chg_bounds(bnd_index, bnd_type, bnd_val);
    } else if (qctype[i] == 'I') {
      _clp_inner_solver.setInteger(mindex[i]);
    } else {
      throw InvalidColTypeException(qctype[i]);
    }
    std::vector<char> colT(1);
    get_col_type(colT.data(), mindex[i], mindex[i]);
  }
}

void SolverCbc::chg_rhs(int id_row, double val) {
  const double *rowLower = _clp_inner_solver.getRowLower();
  const double *rowUpper = _clp_inner_solver.getRowUpper();

  if (rowLower[id_row] <= -COIN_DBL_MAX) {
    if (rowUpper[id_row] >= COIN_DBL_MAX) {
      std::stringstream buffer;
      buffer << "ERROR : unconstrained constraint " << id_row << " in chg_rhs.";
      throw GenericSolverException(buffer.str());
    } else {
      _clp_inner_solver.setRowUpper(id_row, val);
    }
  } else {
    if (rowUpper[id_row] >= COIN_DBL_MAX) {
      _clp_inner_solver.setRowLower(id_row, val);
    } else {
      std::stringstream buffer;
      buffer << "ERROR : constraint " << id_row
             << " has both lower and upper bound in chg_rhs." << std::endl;
      buffer << "Not implemented in CLP interface yet.";
      throw GenericSolverException(buffer.str());
    }
  }
}

void SolverCbc::chg_coef(int id_row, int id_col, double val) {

  // Very tricky method by method "modifyCoefficient" of OsiClp does not work
  CoinPackedMatrix matrix = *_clp_inner_solver.getMatrixByRow();
  const int *column = matrix.getIndices();
  const int *rowLength = matrix.getVectorLengths();
  const CoinBigIndex *rowStart = matrix.getVectorStarts();
  const double *vals = matrix.getElements();

  matrix.modifyCoefficient(id_row, id_col, val);
  _clp_inner_solver.replaceMatrix(matrix);
}

void SolverCbc::chg_row_name(int id_row, std::string const &name) {
  _clp_inner_solver.setRowName(id_row, name);
}

void SolverCbc::chg_col_name(int id_col, std::string const &name) {
  _clp_inner_solver.setColName(id_col, name);
}

/*************************************************************************************************
-----------------------------    Methods to solve the problem
---------------------------------
*************************************************************************************************/
int SolverCbc::solve_lp() {
  int lp_status;

  // Passing OsiClp to Cbc to solve
  // Cbc keeps only solutions of problem
  defineCbcModelFromInnerSolver();

  _cbc.solver()->initialSolve();

  if (_cbc.isInitialSolveProvenOptimal()) {
    lp_status = OPTIMAL;
  } else if (_cbc.isInitialSolveProvenPrimalInfeasible()) {
    lp_status = INFEASIBLE;
  } else if (_cbc.isInitialSolveProvenDualInfeasible()) {
    lp_status = UNBOUNDED;
  } else {
    lp_status = UNKNOWN;
    std::cout << "Error : UNKNOWN CBC STATUS after initial solve." << std::endl;
  }

  return lp_status;
}

int SolverCbc::solve_mip() {
  int lp_status;
  // Passing OsiClp to Cbc to solve
  // Cbc keeps only solutions of problem
  defineCbcModelFromInnerSolver();
  _cbc.branchAndBound();

  /*std::cout << "*********************************************" << std::endl;
  std::cout << "COUCOU CBC STATUS " << _cbc.status() << std::endl;
  std::cout << "COUCOU CBC STATUS " << _cbc.secondaryStatus() << std::endl;
  std::cout << _cbc.isProvenOptimal() << std::endl;
  std::cout << "INFEAS ? " << _cbc.isProvenInfeasible() << std::endl;
  std::cout << "INFEAS ? " << _cbc.isInitialSolveProvenPrimalInfeasible() <<
  std::endl; std::cout << "UNBD   ? " << _cbc.isProvenDualInfeasible() <<
  std::endl; std::cout << "UNBD   ? " <<
  _cbc.isInitialSolveProvenDualInfeasible() << std::endl; std::cout <<
  "*********************************************" << std::endl;*/

  if (_cbc.isProvenOptimal()) {
    if (std::abs(_cbc.solver()->getObjValue()) >= 1e20) {
      lp_status = UNBOUNDED;
    } else {
      lp_status = OPTIMAL;
    }
  } else if (_cbc.isProvenInfeasible() ||
             _cbc.isInitialSolveProvenPrimalInfeasible()) {
    lp_status = INFEASIBLE;
  } else if (_cbc.isProvenDualInfeasible() ||
             _cbc.isInitialSolveProvenDualInfeasible()) {
    lp_status = UNBOUNDED;
  } else {
    lp_status = UNKNOWN;
    std::cout
        << "Error : UNKNOWN CBC STATUS after branch and bound complete search."
        << std::endl;
  }

  return lp_status;
}

/*************************************************************************************************
-------------------------    Methods to get solutions information
-----------------------------
*************************************************************************************************/
void SolverCbc::get_basis(int *rstatus, int *cstatus) const {
  _cbc.solver()->getBasisStatus(cstatus, rstatus);
}

double SolverCbc::get_mip_value() const { return _cbc.getObjValue(); }

double SolverCbc::get_lp_value() const { return _cbc.solver()->getObjValue(); }

int SolverCbc::get_simplex_ite() const {
  return _cbc.solver()->getIterationCount();
}

void SolverCbc::get_lp_sol(double *primals, double *duals,
                           double *reduced_costs) {
  if (primals != NULL) {
    const double *primalSol = _cbc.solver()->getColSolution();
    for (int i(0); i < get_ncols(); i++) {
      primals[i] = primalSol[i];
    }
  }

  if (duals != NULL) {
    const double *dualSol = _cbc.solver()->getRowPrice();
    for (int i(0); i < get_nrows(); i++) {
      duals[i] = dualSol[i];
    }
  }

  if (reduced_costs != NULL) {
    const double *reducedCostSolution = _cbc.solver()->getReducedCost();
    for (int i(0); i < get_ncols(); i++) {
      reduced_costs[i] = reducedCostSolution[i];
    }
  }
}

void SolverCbc::get_mip_sol(double *primals) {
  if (primals != NULL) {
    const double *primalSol = _cbc.getColSolution();
    for (int i(0); i < get_ncols(); i++) {
      primals[i] = primalSol[i];
    }
  }
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels
---------------------------
*************************************************************************************************/
void SolverCbc::set_output_log_level(int loglevel) {
  // Saving asked log_level for calls in solve, when Cbc is reinitialized
  _current_log_level = loglevel;

  _clp_inner_solver.passInMessageHandler(&_message_handler);
  _cbc.passInMessageHandler(&_message_handler);
  if (loglevel > 0) {
    _message_handler.setLogLevel(0, 1); // Coin messages
    _message_handler.setLogLevel(1, 1); // Clp messages
    _message_handler.setLogLevel(2, 1); // Presolve messages
    _message_handler.setLogLevel(3, 1); // Cgl messages
  } else {
    _message_handler.setLogLevel(0, 0); // Coin messages
    _message_handler.setLogLevel(1, 0); // Clp messages
    _message_handler.setLogLevel(2, 0); // Presolve messages
    _message_handler.setLogLevel(3, 0); // Cgl messages
    _message_handler.setLogLevel(0);
  }
}

void SolverCbc::set_algorithm(std::string const &algo) {
  throw InvalidSolverOptionException("set_algorithm : " + algo);
}

void SolverCbc::set_threads(int n_threads) { _cbc.setNumberThreads(n_threads); }

void SolverCbc::set_optimality_gap(double gap) {
  throw InvalidSolverOptionException("set_optimality_gap : " +
                                     std::to_string(gap));
}

void SolverCbc::set_simplex_iter(int iter) {
  throw InvalidSolverOptionException("set_simplex_iter : " +
                                     std::to_string(iter));
}