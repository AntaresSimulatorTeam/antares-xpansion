#include "SolverCbc.h"

#include "COIN_common_functions.h"
using namespace std::literals;

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor
--------------------------------
*************************************************************************************************/
int SolverCbc::_NumberOfProblems = 0;

SolverCbc::SolverCbc(SolverLogManager &log_manager) : SolverCbc() {
  _fp = log_manager.log_file_ptr;
  if (_fp) {
    _clp_inner_solver.messageHandler()->setFilePointer(_fp);
    _cbc.messageHandler()->setFilePointer(_fp);
  }
}
SolverCbc::SolverCbc() {
  _NumberOfProblems += 1;
  set_output_log_level(0);
}

SolverCbc::SolverCbc(const std::shared_ptr<const SolverAbstract> toCopy)
    : SolverCbc() {
  // Try to cast the solver in fictif to a SolverCbc
  if (const auto c = dynamic_cast<const SolverCbc *>(toCopy.get())) {
    _clp_inner_solver = OsiClpSolverInterface(c->_clp_inner_solver);

    defineCbcModelFromInnerSolver();
    _fp = c->_fp;
    if (_fp) {
      _clp_inner_solver.messageHandler()->setFilePointer(_fp);
      _cbc.messageHandler()->setFilePointer(_fp);
    }
  } else {
    _NumberOfProblems -= 1;
    throw InvalidSolverForCopyException(toCopy->get_solver_name(), name_,
                                        LOGLOCATION);
  }
}

SolverCbc::~SolverCbc() {
  _NumberOfProblems -= 1;
  free();
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

void SolverCbc::free() {
  // nothing to do here
}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/
void SolverCbc::write_prob_mps(const std::filesystem::path &filename) {
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

  {
    auto mcol = _clp_inner_solver.getMatrixByCol();
    auto col = *(_clp_inner_solver.getMatrixByCol());
    auto infinity = _clp_inner_solver.getInfinity();
    writer.setMpsData(
        col, infinity, _clp_inner_solver.getColLower(),
        _clp_inner_solver.getColUpper(), _clp_inner_solver.getObjCoefficients(),
        hasInteger ? integrality : nullptr, _clp_inner_solver.getRowLower(),
        _clp_inner_solver.getRowUpper(), colNames, rowNames);
  }

  std::string probName = "";
  _clp_inner_solver.getStrParam(OsiProbName, probName);
  writer.setProblemName(probName.c_str());

  double objOffset = 0.0;
  _clp_inner_solver.getDblParam(OsiObjOffset, objOffset);
  writer.setObjectiveOffset(objOffset);

  writer.writeMps(filename.string().c_str(), 0 /*gzip it*/, 1, 1, nullptr, 0,
                  nullptr);
}

void SolverCbc::chg_col_names(int id_col,
                              const std::vector<std::string> &name) {}
void SolverCbc::chg_row_names(int id_col,
                              const std::vector<std::string> &names) {}

void SolverCbc::write_prob_lp(const std::filesystem::path &filename) {
  _clp_inner_solver.writeLpNative(filename.string().c_str(), nullptr, nullptr);
}

void SolverCbc::write_basis(const std::filesystem::path &filename) {
  auto clp_updated_solver_ptr =
      dynamic_cast<OsiClpSolverInterface *>(_cbc.solver());

  ClpSimplex *clps = clp_updated_solver_ptr->getModelPtr();

  setClpSimplexColNamesFromInnerSolver(clps);
  setClpSimplexRowNamesFromInnerSolver(clps);
  auto filename_str = filename.string();
  auto filename_c_str = filename_str.c_str();

  /*
  formatType must be = 0 (normal accuracy) to avoid undefined behavior.
  see Adr in
  conception/Architecture_decision_records/Change_Solver_Basis_Format.md
  */
  int status = clps->writeBasis(filename_c_str, true, 0);
  zero_status_check(status, "write basis", LOGLOCATION);
}

void SolverCbc::setClpSimplexColNamesFromInnerSolver(ClpSimplex *clps) const {
  for (int col_id(0); col_id < get_ncols(); col_id++) {
    std::string name = _clp_inner_solver.getColName(col_id);
    clps->setColumnName(col_id, name);
  }
}

void SolverCbc::setClpSimplexRowNamesFromInnerSolver(ClpSimplex *clps) const {
  for (int row_id(0); row_id < clps->getNumRows(); row_id++) {
    std::string name = _clp_inner_solver.getRowName(row_id);
    clps->setRowName(row_id, name);
  }
}

void SolverCbc::read_prob_mps(const std::filesystem::path &prob_name) {
  int status = _clp_inner_solver.readMps(prob_name.string().c_str());
  zero_status_check(status, " read problem "s + prob_name.string(),
                    LOGLOCATION);
  defineCbcModelFromInnerSolver();
}

void SolverCbc::read_prob_lp(const std::filesystem::path &prob_name) {
  int status = _clp_inner_solver.readLp(prob_name.string().c_str());
  zero_status_check(status, "read problem", LOGLOCATION);
  defineCbcModelFromInnerSolver();
}

void SolverCbc::read_basis(const std::filesystem::path &filename) {
  int status =
      _clp_inner_solver.getModelPtr()->readBasis(filename.string().c_str());
  // readBasis returns 1 if successful
  zero_status_check(status - 1, "read basis", LOGLOCATION);
  defineCbcModelFromInnerSolver();
}

void SolverCbc::copy_prob(const SolverAbstract::Ptr fictif_solv) {
  auto error = LOGLOCATION + "Copy CBC problem : TO DO WHEN NEEDED";
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
  const double *internalObj = _clp_inner_solver.getObjCoefficients();

  for (int i = first; i < last + 1; i++) {
    obj[i - first] = internalObj[i];
  }
}

void SolverCbc::set_obj_to_zero() {
  auto ncols = get_ncols();
  std::vector<double> zeros_val(ncols, 0.0);
  _clp_inner_solver.setObjective(zeros_val.data());
}

void SolverCbc::set_obj(const double *obj, int first, int last) {
  if (last - first + 1 == get_ncols()) {
    _clp_inner_solver.setObjective(obj);
  } else {
    for (int index = first; index < last + 1; ++index) {
      _clp_inner_solver.setObjCoeff(index, obj[index]);
    }
  }
}

void SolverCbc::get_rows(int *mstart, int *mclind, double *dmatval, int size,
                         int *nels, int first, int last) const {
  CoinPackedMatrix matrix = *_clp_inner_solver.getMatrixByRow();
  coin_common::fill_rows_from_COIN_matrix(matrix, mstart, mclind, dmatval, nels,
                                          first, last);
}

void SolverCbc::get_row_type(char *qrtype, int first, int last) const {
  const double *rowLower = _clp_inner_solver.getRowLower();
  const double *rowUpper = _clp_inner_solver.getRowUpper();
  coin_common::fill_row_type_from_row_bounds(rowLower, rowUpper, qrtype, first,
                                             last);
}

void SolverCbc::get_rhs(double *rhs, int first, int last) const {
  const double *rowLower = _clp_inner_solver.getRowLower();
  const double *rowUpper = _clp_inner_solver.getRowUpper();
  coin_common::fill_rhs_from_bounds(rowLower, rowUpper, rhs, first, last);
}

void SolverCbc::get_rhs_range(double *range, int first, int last) const {
  std::stringstream buffer;
  buffer << LOGLOCATION
         << "ERROR : get rhs range not implemented in the interface for COIN "
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

// TODO update see SolverCbc::get_col_index
int SolverCbc::get_row_index(std::string const &name) {
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

int SolverCbc::get_col_index(std::string const &name) {
  const auto &col_names = get_col_names();
  const auto index = std::distance(
      col_names.begin(), std::find(col_names.cbegin(), col_names.cend(), name));

  if (index >= col_names.end() - col_names.begin()) {
    return -1;
  } else {
    return index;
  }
}

std::vector<std::string> SolverCbc::get_row_names(int first, int last) {
  int size = 1 + last - first;
  std::vector<std::string> names;
  names.reserve(size);

  std::vector<std::string> solver_row_names = _clp_inner_solver.getRowNames();
  if (solver_row_names.size() < size) {
    throw InvalidRowSizeException(size, solver_row_names.size(), LOGLOCATION);
  }

  for (int i(first); i < last + 1; i++) {
    names.push_back(solver_row_names[i]);
  }
  return names;
}
std::vector<std::string> SolverCbc::get_row_names() {
  return _clp_inner_solver.getRowNames();
}

std::vector<std::string> SolverCbc::get_col_names(int first, int last) {
  int size = 1 + last - first;
  std::vector<std::string> names;
  names.reserve(size);

  std::vector<std::string> solver_col_names = _clp_inner_solver.getColNames();
  if (solver_col_names.size() < size) {
    throw InvalidColSizeException(size, solver_col_names.size(), LOGLOCATION);
  }

  for (int i(first); i < last + 1; i++) {
    names.push_back(solver_col_names[i]);
  }
  return names;
}
std::vector<std::string> SolverCbc::get_col_names() {
  return _clp_inner_solver.getColNames();
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
                         const double *dmatval,
                         const std::vector<std::string> &row_names) {
  std::vector<double> rowLower(newrows);
  std::vector<double> rowUpper(newrows);
  int nrowInit = get_nrows();

  coin_common::fill_row_bounds_from_new_rows_data(rowLower, rowUpper, newrows,
                                                  qrtype, rhs);
  _clp_inner_solver.addRows(newrows, mstart, mclind, dmatval, rowLower.data(),
                            rowUpper.data());
  if (row_names.size() > 0) {
    int nrowFinal = get_nrows();
    for (int i = nrowInit; i < nrowFinal; i++) {
      chg_row_name(i, row_names[i - nrowInit]);
    }
  }
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
  auto error =
      LOGLOCATION + "ERROR : addnames not implemented in the CLP interface.";
  throw NotImplementedFeatureSolverException(error);
}

void SolverCbc::add_names(int type, const std::vector<std::string> &cnames,
                          int first, int end) {
  // TODO
  auto error =
      LOGLOCATION + "ERROR : addnames not implemented in the CLP interface.";
  throw NotImplementedFeatureSolverException(error);
}

void SolverCbc::chg_obj(const std::vector<int> &mindex,
                        const std::vector<double> &obj) {
  assert(obj.size() == mindex.size());
  for (int i(0); i < obj.size(); i++) {
    _clp_inner_solver.setObjCoeff(mindex[i], obj[i]);
  }
}

void SolverCbc::chg_obj_direction(const bool minimize) {
  int objsense = minimize ? 1 : -1;
  _clp_inner_solver.setObjSense(objsense);
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
      throw InvalidBoundTypeException(qbtype[i], LOGLOCATION);
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
      throw InvalidColTypeException(qctype[i], LOGLOCATION);
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
      buffer << LOGLOCATION << "ERROR : unconstrained constraint " << id_row
             << " in chg_rhs.";
      throw GenericSolverException(buffer.str());
    } else {
      _clp_inner_solver.setRowUpper(id_row, val);
    }
  } else {
    if (rowUpper[id_row] >= COIN_DBL_MAX) {
      _clp_inner_solver.setRowLower(id_row, val);
    } else {
      std::stringstream buffer;
      buffer << LOGLOCATION << "ERROR : constraint " << id_row
             << " has both lower and upper bound in chg_rhs." << std::endl;
      buffer << "Not implemented in CLP interface yet.";
      throw GenericSolverException(buffer.str());
    }
  }
}

void SolverCbc::chg_coef(int id_row, int id_col, double val) {
  // Very tricky method by method "modifyCoefficient" of OsiClp does not work
  CoinPackedMatrix matrix = *_clp_inner_solver.getMatrixByRow();

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

void SolverCbc::SetBasis(std::vector<int> rstatus, std::vector<int> cstatus) {
  _cbc.solver()->setBasisStatus(rstatus.data(), cstatus.data());
}

double SolverCbc::get_mip_value() const { return _cbc.getObjValue(); }

double SolverCbc::get_lp_value() const { return _cbc.solver()->getObjValue(); }

int SolverCbc::get_splex_num_of_ite_last() const {
  return _cbc.solver()->getIterationCount();
}

void SolverCbc::get_lp_sol(double *primals, double *duals,
                           double *reduced_costs) {
  if (primals != nullptr) {
    const double *primalSol = _cbc.solver()->getColSolution();
    for (int i(0); i < get_ncols(); i++) {
      primals[i] = primalSol[i];
    }
  }

  if (duals != nullptr) {
    const double *dualSol = _cbc.solver()->getRowPrice();
    for (int i(0); i < get_nrows(); i++) {
      duals[i] = dualSol[i];
    }
  }

  if (reduced_costs != nullptr) {
    const double *reducedCostSolution = _cbc.solver()->getReducedCost();
    for (int i(0); i < get_ncols(); i++) {
      reduced_costs[i] = reducedCostSolution[i];
    }
  }
}

void SolverCbc::get_mip_sol(double *primals) {
  if (primals != nullptr) {
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

  for (const auto message_handler :
       {_clp_inner_solver.messageHandler(), _cbc.messageHandler()}) {
    if (loglevel > 1 && _fp) {
      message_handler->setLogLevel(0, 1);  // Coin messages
      message_handler->setLogLevel(1, 1);  // Clp messages
      message_handler->setLogLevel(2, 1);  // Presolve messages
      message_handler->setLogLevel(3, 1);  // Cgl messages
    } else {
      message_handler->setLogLevel(0, 0);  // Coin messages
      message_handler->setLogLevel(1, 0);  // Clp messages
      message_handler->setLogLevel(2, 0);  // Presolve messages
      message_handler->setLogLevel(3, 0);  // Cgl messages
      message_handler->setLogLevel(0);
    }
  }
}

void SolverCbc::set_algorithm(std::string const &algo) {
  throw InvalidSolverOptionException("set_algorithm : " + algo, LOGLOCATION);
}

void SolverCbc::set_threads(int n_threads) { _cbc.setNumberThreads(n_threads); }

void SolverCbc::set_optimality_gap(double gap) {
  throw InvalidSolverOptionException(
      "set_optimality_gap : " + std::to_string(gap), LOGLOCATION);
}

void SolverCbc::set_simplex_iter(int iter) {
  throw InvalidSolverOptionException(
      "set_simplex_iter : " + std::to_string(iter), LOGLOCATION);
}
