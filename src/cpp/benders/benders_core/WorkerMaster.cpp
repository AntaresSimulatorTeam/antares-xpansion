#include "WorkerMaster.h"

#include "glog/logging.h"
#include "solver_utils.h"

WorkerMaster::WorkerMaster() { _is_master = true; }

/*!
 *  \brief Constructor of a Master Problem
 *
 *  Construct a Master Problem by loading mps and mapping files and adding the
 * variable alpha
 *
 *  \param variable_map : map linking each variable to its id in the problem
 *  \param path_to_mps : path to the problem mps file
 *  \param solver_name : solver name
 *  \param log_level : solver log level
 *  \param subproblems_count : number of subproblems
 */
WorkerMaster::WorkerMaster(VariableMap const &variable_map,
                           const std::filesystem::path &path_to_mps,
                           const std::string &solver_name, const int log_level,
                           int subproblems_count,
                           const std::filesystem::path &log_name)
    : Worker(), subproblems_count(subproblems_count) {
  _is_master = true;
  init(variable_map, path_to_mps, solver_name, log_level, log_name);

  _set_upper_bounds();
  _add_alpha_var();
}
WorkerMaster::~WorkerMaster() {}

/*!
 *  \brief Return optimal variables of a problem
 *
 *  Set optimal variables of a problem which has the form (min(x,alpha) : f(x) +
 * alpha)
 *
 *  \param x0 : reference to an empty map list
 *
 *  \param alpha : reference to an empty double
 */
void WorkerMaster::get(Point &x0, double &alpha, DblVector &alpha_i) {
  x0.clear();
  std::vector<double> ptr(_solver->get_ncols());

  if (_solver->get_n_integer_vars() > 0) {
    _solver->get_mip_sol(ptr.data());
  } else {
    _solver->get_lp_sol(ptr.data(), NULL, NULL);
  }
  assert(_id_alpha_i.back() + 1 == ptr.size());
  for (auto const &kvp : _id_to_name) {
    x0[kvp.second] = ptr[kvp.first];
  }
  alpha = ptr[_id_alpha];
  for (int i(0); i < _id_alpha_i.size(); ++i) {
    alpha_i[i] = ptr[_id_alpha_i[i]];
  }
}

/*!
 *  \brief Set dual values of a problem in a vector
 *
 *  \param dual : reference to a vector of double
 */
void WorkerMaster::get_dual_values(std::vector<double> &dual) const {
  dual.resize(get_number_constraint());
  solver_getlpdual(_solver, dual);
}

/*!
 *  \brief Return number of constraint in a problem
 */
int WorkerMaster::get_number_constraint() const { return _solver->get_nrows(); }

/*!
 *  \brief Delete nrows last rows of a problem
 *
 *  \param nrows : number of rows to delete
 */
void WorkerMaster::delete_constraint(int const nrows) const {
  std::vector<int> mindex(nrows, 0);
  int const nconstraint(get_number_constraint());
  for (int i(0); i < nrows; i++) {
    mindex[i] = nconstraint - nrows + i;
  }
  solver_deactivaterows(_solver, mindex);
}

/*!
 *  \brief Add benders cut to a problem
 *
 *  \param s : optimal slave variables
 *  \param x0 : optimal Master variables
 *  \param rhs : optimal slave value
 */
void WorkerMaster::add_cut(Point const &s, Point const &x0,
                           double const &rhs) const {
  // cut is -rhs >= alpha  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_with_master_variable(s, x0, rhs, rowrhs);
  define_matval_mclind(s, matval, mclind);

  solver_addrows(_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::define_rhs_with_master_variable(
    const Point &s, const Point &x0, const double &rhs,
    std::vector<double> &rowrhs) const {
  rowrhs.front() -= rhs;
  for (auto const &kvp : _name_to_id) {
    if (s.find(kvp.first) != s.end()) {
      rowrhs.front() +=
          (s.find(kvp.first)->second * x0.find(kvp.first)->second);
    }
  }
}

void WorkerMaster::define_matval_mclind(const Point &s,
                                        std::vector<double> &matval,
                                        std::vector<int> &mclind) const {
  size_t mclindCnt_l(0);
  for (auto const &kvp : _name_to_id) {
    if (s.find(kvp.first) != s.end()) {
      mclind[mclindCnt_l] = kvp.second;
      matval[mclindCnt_l] = s.find(kvp.first)->second;
      ++mclindCnt_l;
    }
  }
  mclind.back() = _id_alpha;
  matval.back() = -1;
}

/*!
 *  \brief Add benders cut to a problem
 *
 *  \param s : optimal slave variables
 *  \param sx0 : subgradient times x0
 *  \param rhs : optimal slave value
 */
void WorkerMaster::add_dynamic_cut(Point const &s, double const &sx0,
                                   double const &rhs) const {
  // cut is -rhs >= alpha  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_from_sx0(sx0, rhs, rowrhs);
  define_matval_mclind(s, matval, mclind);
  solver_addrows(_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::define_rhs_from_sx0(const double &sx0, const double &rhs,
                                       std::vector<double> &rowrhs) const {
  rowrhs.front() -= rhs;
  rowrhs.front() += sx0;
}

/*!
 *  \brief Add benders cut to a problem
 *
 *  \param i : identifier of a subproblem
 *  \param s : optimal slave variables
 *  \param sx0 : subgradient times x0
 *  \param rhs : optimal slave value
 */
void WorkerMaster::add_cut_by_iter(int const i, Point const &s,
                                   double const &sx0, double const &rhs) const {
  // cut is -rhs >= alpha  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_from_sx0(sx0, rhs, rowrhs);
  define_matval_mclind_for_index(i, s, matval, mclind);

  solver_addrows(_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::define_matval_mclind_for_index(
    const int i, const Point &s, std::vector<double> &matval,
    std::vector<int> &mclind) const {
  size_t mclindCnt_l(0);
  for (auto const &kvp : _name_to_id) {
    if (s.find(kvp.first) != s.end()) {
      mclind[mclindCnt_l] = kvp.second;
      matval[mclindCnt_l] = s.find(kvp.first)->second;
      ++mclindCnt_l;
    }
  }
  mclind.back() = _id_alpha_i[i];
  matval.back() = -1;
}

/*!
 *  \brief Add one benders cut to a problem
 *
 *  \param i : identifier of a subproblem
 *  \param s : optimal slave variables
 *  \param x0 : optimal Master variables
 *  \param rhs : optimal slave value
 */
void WorkerMaster::addSubproblemCut(int i, Point const &s, Point const &x0,
                                    double const &rhs) const {
  // cut is -rhs >= alpha  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_with_master_variable(s, x0, rhs, rowrhs);
  define_matval_mclind_for_index(i, s, matval, mclind);

  solver_addrows(_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::_set_upper_bounds() const {
  // Cbc solver sets infinite upper bounds to DBL_MAX = 1.79769e+308 which is
  // way too large as it appears in datas.max_invest. We set it to 1e20
  int ncols = _solver->get_ncols();
  DblVector bounds(ncols);
  _solver->get_ub(bounds.data(), 0, ncols - 1);
  CharVector bndTypes(ncols, 'U');
  IntVector indices(ncols);
  for (int i = 0; i < _solver->get_ncols(); i++) {
    indices[i] = i;
    bounds[i] = std::min(bounds[i], 1e20);
  }
  _solver->chg_bounds(indices, bndTypes, bounds);
}

void WorkerMaster::_add_alpha_var() {
  // add the variable alpha
  auto const it(_name_to_id.find("alpha"));
  if (it == _name_to_id.end()) {
    double lb(-1e10); /*!< Lower Bound */
    double ub(+1e20); /*!< Upper Bound*/
    double obj(+1);
    std::vector<int> start(2, 0);
    _id_alpha =
        _solver->get_ncols(); /* Set the number of columns in _id_alpha */

    solver_addcols(
        _solver, DblVector(1, obj), IntVector(1, 0), IntVector(0, 0),
        DblVector(0, 0.0), DblVector(1, lb), DblVector(1, ub),
        CharVector(1, 'C'),
        StrVector(1, "alpha")); /* Add variable alpha and its parameters */

    _id_alpha_i.resize(subproblems_count, -1);
    for (int i(0); i < subproblems_count; ++i) {
      std::stringstream buffer;
      buffer << "alpha_" << i;
      _id_alpha_i[i] = _solver->get_ncols();
      solver_addcols(
          _solver, DblVector(1, 0.0), IntVector(1, 0), IntVector(0, 0),
          DblVector(0, 0.0), DblVector(1, lb), DblVector(1, ub),
          CharVector(1, 'C'),
          StrVector(
              1, buffer.str())); /* Add variable alpha_i and its parameters */
    }

    std::vector<char> rowtype = {'E'};
    std::vector<double> rowrhs = {0};
    std::vector<int> mstart = {0, subproblems_count + 1};
    std::vector<double> matval(subproblems_count + 1, 0);
    std::vector<int> mclind(subproblems_count + 1);
    mclind[0] = _id_alpha;
    matval[0] = 1;
    for (int i(0); i < subproblems_count; ++i) {
      mclind[i + 1] = _id_alpha_i[i];
      matval[i + 1] = -1;
    }

    solver_addrows(_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
  } else { /* resume mode*/
    // _id_alpha = _solver.get_col_index("alpha");
    LOG(INFO) << "ERROR a variable named alpha is in input" << std::endl;
  }
}

/*!
 *  \brief Fix an upper bound and the variable alpha of a problem
 *
 *  \param bestUB : bound to fix
 */
void WorkerMaster::fix_alpha(double const &bestUB) const {
  std::vector<int> mindex(1, _id_alpha);
  std::vector<char> bnd_types(1, 'U');
  std::vector<double> bnd_values(1, bestUB);
  _solver->chg_bounds(mindex, bnd_types, bnd_values);
}
