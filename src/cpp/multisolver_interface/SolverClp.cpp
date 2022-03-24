#include "SolverClp.h"

#include "COIN_common_functions.h"

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor
--------------------------------
*************************************************************************************************/
int SolverClp::_NumberOfProblems = 0;

SolverClp::SolverClp(const std::string &log_file) : SolverClp() {
  _log_file = log_file;
  if (_log_file.empty()) {
    std::cout << "Empty log file name, fallback to default behaviour"
              << std::endl;
  } else {
    if ((_fp = fopen(_log_file.c_str(), "a+")) == NULL) {
      std::cerr << "Invalid log file name passed as parameter: " << _log_file
                << std::endl;
    } else {
      setvbuf(_fp, NULL, _IONBF, 0);
      _message_handler.setFilePointer(_fp);
    }
  }
}
SolverClp::SolverClp() {
  _NumberOfProblems += 1;
  _fp = nullptr;
}

SolverClp::SolverClp(const std::shared_ptr<const SolverAbstract> toCopy)
    : SolverClp() {
  _fp = nullptr;
  // Try to cast the solver in fictif to a SolverCPLEX
  if (const auto c = dynamic_cast<const SolverClp *>(toCopy.get())) {
    _clp = ClpSimplex(c->_clp);
    _log_file = toCopy->_log_file;
    _fp = fopen(_log_file.c_str(), "a+");
    if (_fp != nullptr) {
      setvbuf(_fp, nullptr, _IONBF, 0);
      _message_handler.setFilePointer(_fp);
    }
  } else {
    _NumberOfProblems -= 1;
    throw InvalidSolverForCopyException(toCopy->get_solver_name(),
                                        get_solver_name());
  }
}

SolverClp::~SolverClp() {
  _NumberOfProblems -= 1;
  if (_fp != nullptr) {
    fclose(_fp);
    _fp = nullptr;
  }
  free();
}

int SolverClp::get_number_of_instances() { return _NumberOfProblems; }

/*************************************************************************************************
-----------------------------------    Output stream management
------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments
---------------
*************************************************************************************************/
void SolverClp::init() { _clp = ClpSimplex(); }

void SolverClp::free() {
  //_clp = ClpSimplex();
}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/
void SolverClp::write_prob_mps(const std::string &filename) {
  _clp.writeMps(filename.c_str(), 1);
}

void SolverClp::write_prob_lp(const std::string &filename) {
  _clp.writeLp(filename.c_str());
}

void SolverClp::read_prob_mps(const std::string &filename) {
  _clp.readMps(filename.c_str(), true, false);
}

void SolverClp::read_prob_lp(const std::string &filename) {
  _clp.readLp(filename.c_str());
}

void SolverClp::copy_prob(const SolverAbstract::Ptr fictif_solv) {
  std::string error = "Copy Clp problem : TO DO WHEN NEEDED";
  throw NotImplementedFeatureSolverException(error);
}

/*************************************************************************************************
-----------------------    Get general informations about problem
----------------------------
*************************************************************************************************/
int SolverClp::get_ncols() const { return _clp.getNumCols(); }

int SolverClp::get_nrows() const { return _clp.getNumRows(); }

int SolverClp::get_nelems() const { return _clp.getNumElements(); }

int SolverClp::get_n_integer_vars() const {
  int nvars = get_ncols();
  int n_int_vars(0);
  for (int i(0); i < nvars; i++) {
    if (_clp.isInteger(i)) {
      n_int_vars += 1;
    }
  }
  return n_int_vars;
}

void SolverClp::get_obj(double *obj, int first, int last) const {
  double *internalObj = _clp.objective();

  for (int i = first; i < last + 1; i++) {
    obj[i - first] = internalObj[i];
  }
}

void SolverClp::get_rows(int *mstart, int *mclind, double *dmatval, int size,
                         int *nels, int first, int last) const {
  CoinPackedMatrix matrix = *_clp.matrix();
  matrix.reverseOrdering();
  coin_common::fill_rows_from_COIN_matrix(matrix, mstart, mclind, dmatval, nels,
                                          first, last);
}

void SolverClp::get_row_type(char *qrtype, int first, int last) const {
  const double *rowLower = _clp.getRowLower();
  const double *rowUpper = _clp.getRowUpper();

  coin_common::fill_row_type_from_row_bounds(rowLower, rowUpper, qrtype, first,
                                             last);
}

void SolverClp::get_rhs(double *rhs, int first, int last) const {
  const double *rowLower = _clp.getRowLower();
  const double *rowUpper = _clp.getRowUpper();
  coin_common::fill_rhs_from_bounds(rowLower, rowUpper, rhs, first, last);
}

void SolverClp::get_rhs_range(double *range, int first, int last) const {
  std::string error =
      "ERROR : get rhs range not implemented for COIN CLP interface";
  throw NotImplementedFeatureSolverException(error);
}

void SolverClp::get_col_type(char *coltype, int first, int last) const {
  const double *colLower = _clp.getColLower();
  const double *colUpper = _clp.getColUpper();

  for (int i = first; i < last + 1; i++) {
    if (_clp.isInteger(i)) {
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

void SolverClp::get_lb(double *lb, int first, int last) const {
  const double *colLower = _clp.getColLower();

  for (int i = first; i < last + 1; i++) {
    lb[i - first] = colLower[i];
  }
}

void SolverClp::get_ub(double *ub, int first, int last) const {
  const double *colUpper = _clp.getColUpper();

  for (int i = first; i < last + 1; i++) {
    ub[i - first] = colUpper[i];
  }
}

int SolverClp::get_row_index(std::string const &name) const {
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

int SolverClp::get_col_index(std::string const &name) const {
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

std::vector<std::string> SolverClp::get_row_names(int first, int last) {
  std::vector<std::string> names;
  names.reserve(1 + last - first);

  for (int i = first; i < last + 1; i++) {
    names.push_back(_clp.getRowName(i));
  }
  return names;
}

std::vector<std::string> SolverClp::get_col_names(int first, int last) {
  std::vector<std::string> names;
  names.reserve(1 + last - first);

  for (int i = first; i < last + 1; i++) {
    names.push_back(_clp.getColumnName(i));
  }
  return names;
}

/*************************************************************************************************
------------------------------    Methods to modify problem
----------------------------------
*************************************************************************************************/

void SolverClp::del_rows(int first, int last) {
  std::vector<int> mindex(last - first + 1);
  for (int i = 0; i < last - first + 1; i++) {
    mindex[i] = first + i;
  }
  _clp.deleteRows(last - first + 1, mindex.data());
}

void SolverClp::add_rows(int newrows, int newnz, const char *qrtype,
                         const double *rhs, const double *range,
                         const int *mstart, const int *mclind,
                         const double *dmatval) {
  std::vector<double> rowLower(newrows);
  std::vector<double> rowUpper(newrows);
  coin_common::fill_row_bounds_from_new_rows_data(rowLower, rowUpper, newrows,
                                                  qrtype, rhs);

  _clp.addRows(newrows, rowLower.data(), rowUpper.data(), mstart, mclind,
               dmatval);
}

void SolverClp::add_cols(int newcol, int newnz, const double *objx,
                         const int *mstart, const int *mrwind,
                         const double *dmatval, const double *bdl,
                         const double *bdu) {
  std::vector<int> colStart(newcol + 1);
  for (int i(0); i < newcol; i++) {
    colStart[i] = mstart[i];
  }
  colStart[newcol] = newnz;

  _clp.addColumns(newcol, bdl, bdu, objx, colStart.data(), mrwind, dmatval);
}

void SolverClp::add_name(int type, const char *cnames, int indice) {
  std::string error = "ERROR : addnames not implemented in the CLP interface.";
  throw NotImplementedFeatureSolverException(error);
}

void SolverClp::chg_obj(const std::vector<int> &mindex,
                        const std::vector<double> &obj) {
  assert(obj.size() == mindex.size());
  for (int i(0); i < obj.size(); i++) {
    _clp.setObjCoeff(mindex[i], obj[i]);
  }
}

void SolverClp::chg_obj_direction(const bool minimize) {
  int objsense = minimize ? 1 : -1;
  _clp.setOptimizationDirection(objsense);
}

void SolverClp::chg_bounds(const std::vector<int> &mindex,
                           const std::vector<char> &qbtype,
                           const std::vector<double> &bnd) {
  assert(qbtype.size() == mindex.size());
  assert(bnd.size() == mindex.size());
  for (int i(0); i < mindex.size(); i++) {
    if (qbtype[i] == 'L') {
      _clp.setColLower(mindex[i], bnd[i]);
    } else if (qbtype[i] == 'U') {
      _clp.setColUpper(mindex[i], bnd[i]);
    } else if (qbtype[i] == 'B') {
      _clp.setColLower(mindex[i], bnd[i]);
      _clp.setColUpper(mindex[i], bnd[i]);
    } else {
      throw InvalidBoundTypeException(qbtype[i]);
    }
  }
}

void SolverClp::chg_col_type(const std::vector<int> &mindex,
                             const std::vector<char> &qctype) {
  assert(qctype.size() == mindex.size());
  std::vector<int> bnd_index(1, 0);
  std::vector<char> bnd_type(1, 'U');
  std::vector<double> bnd_val(1, 1.0);

  for (int i = 0; i < mindex.size(); i++) {
    switch (qctype[i]) {
      case 'C':
        _clp.setContinuous(mindex[i]);
        break;
      case 'B':
        _clp.setInteger(mindex[i]);
        bnd_index[0] = mindex[i];
        chg_bounds(bnd_index, bnd_type, bnd_val);
        break;
      case 'I':
        _clp.setInteger(mindex[i]);
        break;
      default:
        throw InvalidColTypeException(qctype[i]);
    }
  }
}

void SolverClp::chg_rhs(int id_row, double val) {
  const double *rowLower = _clp.getRowLower();
  const double *rowUpper = _clp.getRowUpper();

  if (rowLower[id_row] <= -COIN_DBL_MAX) {
    if (rowUpper[id_row] >= COIN_DBL_MAX) {
      std::stringstream buffer;
      buffer << "ERROR : unconstrained constraint " << id_row << " in chg_rhs.";
      throw GenericSolverException(buffer.str());
    } else {
      _clp.setRowUpper(id_row, val);
    }
  } else {
    if (rowUpper[id_row] >= COIN_DBL_MAX) {
      _clp.setRowLower(id_row, val);
    } else {
      std::stringstream buffer;
      std::cout << "ERROR : constraint " << id_row
                << " has both different lower and upper bound in chg_rhs.";
      throw GenericSolverException(buffer.str());
    }
  }
}

void SolverClp::chg_coef(int id_row, int id_col, double val) {
  CoinPackedMatrix *matrix = _clp.matrix();
  matrix->modifyCoefficient(id_row, id_col, val);
}

void SolverClp::chg_row_name(int id_row, std::string const &name) {
  std::string copy_name = name;
  _clp.setRowName(id_row, copy_name);
}

void SolverClp::chg_col_name(int id_col, std::string const &name) {
  std::string copy_name = name;
  _clp.setColumnName(id_col, copy_name);
}

/*************************************************************************************************
-----------------------------    Methods to solve the problem
---------------------------------
*************************************************************************************************/
int SolverClp::solve_lp() {
  int lp_status;
  _clp.dual();

  int clp_status = _clp.status();
  if (clp_status == CLP_OPTIMAL) {
    lp_status = OPTIMAL;
  } else if (clp_status == CLP_PRIMAL_INFEASIBLE) {
    lp_status = INFEASIBLE;
  } else if (clp_status == CLP_DUAL_INFEASIBLE) {
    lp_status = UNBOUNDED;
  } else {
    lp_status = UNKNOWN;
    std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
  }
  return lp_status;
}

int SolverClp::solve_mip() {
  int lp_status;
  _clp.dual();

  int clp_status = _clp.status();
  if (clp_status == CLP_OPTIMAL) {
    lp_status = OPTIMAL;
  } else if (clp_status == CLP_PRIMAL_INFEASIBLE) {
    lp_status = INFEASIBLE;
  } else if (clp_status == CLP_DUAL_INFEASIBLE) {
    lp_status = UNBOUNDED;
  } else {
    lp_status = UNKNOWN;
    std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
  }
  return lp_status;
}

/*************************************************************************************************
-------------------------    Methods to get solutions information
-----------------------------
*************************************************************************************************/
void SolverClp::get_basis(int *rstatus, int *cstatus) const {
  int ncols = get_ncols();
  for (int i = 0; i < ncols; i++) {
    cstatus[i] = _clp.getColumnStatus(i);
  }

  int nrows = get_nrows();
  for (int i = 0; i < nrows; i++) {
    rstatus[i] = _clp.getRowStatus(i);
  }
}

double SolverClp::get_mip_value() const { return _clp.objectiveValue(); }

double SolverClp::get_lp_value() const { return _clp.objectiveValue(); }

int SolverClp::get_splex_num_of_ite_last() const {
  return _clp.numberIterations();
}

void SolverClp::get_lp_sol(double *primals, double *duals,
                           double *reduced_costs) {
  if (primals != NULL) {
    double *primalSol = _clp.primalColumnSolution();
    for (int i(0); i < get_ncols(); i++) {
      primals[i] = primalSol[i];
    }
  }

  if (duals != NULL) {
    double *dualSol = _clp.dualRowSolution();
    for (int i(0); i < get_nrows(); i++) {
      duals[i] = dualSol[i];
    }
  }

  if (reduced_costs != NULL) {
    double *reducedCostSolution = _clp.dualColumnSolution();
    for (int i(0); i < get_ncols(); i++) {
      reduced_costs[i] = reducedCostSolution[i];
      // std::cout << "RD    " << i << " " << reducedCostSolution[i] <<
      // std::endl;
    }
  }
}

void SolverClp::get_mip_sol(double *primals) {
  if (primals != NULL) {
    double *primalSol = _clp.primalColumnSolution();
    for (int i(0); i < get_ncols(); i++) {
      primals[i] = primalSol[i];
    }
  }
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels
---------------------------
*************************************************************************************************/
void SolverClp::set_output_log_level(int loglevel) {
  _clp.passInMessageHandler(&_message_handler);
  if (loglevel > 0) {
    _message_handler.setLogLevel(1);
  } else {
    _message_handler.setLogLevel(0);
  }
}

void SolverClp::set_algorithm(std::string const &algo) {
  if (algo == "DUAL") {
    _clp.setAlgorithm(1);
  } else {
    throw InvalidSolverOptionException("set_algorithm : " + algo);
  }
}

void SolverClp::set_threads(int n_threads) { _clp.setNumberThreads(n_threads); }

void SolverClp::set_optimality_gap(double gap) {
  throw InvalidSolverOptionException("set_optimality_gap : " +
                                     std::to_string(gap));
}

void SolverClp::set_simplex_iter(int iter) { _clp.setMaximumIterations(iter); }
