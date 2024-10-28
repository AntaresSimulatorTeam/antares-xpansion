#include "antares-xpansion/benders/benders_core/WorkerMaster.h"


#include "antares-xpansion/helpers/solver_utils.h"

WorkerMaster::WorkerMaster(Logger logger) : Worker(logger) {
  _is_master = true;
}

/*!
 *  \brief Constructor of a Master Problem
 *
 *  Construct a Master Problem by loading mps and mapping files and adding the
 * variable overall_subpb_cost_under_approx
 *
 *  \param variable_map : map linking each variable to its id in the problem
 *  \param path_to_mps : path to the problem mps file
 *  \param solver_name : solver name
 *  \param log_level : solver log level
 *  \param subproblems_count : number of subproblems
 */
WorkerMaster::WorkerMaster(
    VariableMap const &variable_map, const std::filesystem::path &path_to_mps,
    const std::string &solver_name, const int log_level, int subproblems_count,
    SolverLogManager&solver_log_manager,
    const bool mps_has_alpha, Logger logger)
    : Worker(std::move(logger)),
      subproblems_count(subproblems_count),
      _mps_has_alpha(mps_has_alpha) {
  _is_master = true;

  init(variable_map, path_to_mps, solver_name, log_level, solver_log_manager);
  if (!_mps_has_alpha) {
    _set_upper_bounds();
  }
  _set_alpha_var();
  _set_nb_units_var_ids();
}

/*!
 *  \brief Return optimal variables of a problem
 *
 *  Set optimal variables of a problem which has the form
 * (min(x,overall_subpb_cost_under_approx) : f(x) +
 * overall_subpb_cost_under_approx)
 *
 *  \param x_out : reference to an empty map list
 *
 *  \param overall_subpb_cost_under_approx : reference to an empty double
 */
void WorkerMaster::get(Point &x_out, double &overall_subpb_cost_under_approx,
                       DblVector &single_subpb_costs_under_approx) {
  x_out.clear();
  std::vector<double> ptr(_solver->get_ncols());

  if (_solver->get_n_integer_vars() > 0) {
    _solver->get_mip_sol(ptr.data());
  } else {
    _solver->get_lp_sol(ptr.data(), nullptr, nullptr);
  }
  assert(id_single_subpb_costs_under_approx_.back() + 1 == ptr.size());
  for (auto const &kvp : _id_to_name) {
    x_out[kvp.second] = ptr[kvp.first];
  }
  overall_subpb_cost_under_approx = ptr[_id_alpha];
  for (int i(0); i < id_single_subpb_costs_under_approx_.size(); ++i) {
    single_subpb_costs_under_approx[i] =
        ptr[id_single_subpb_costs_under_approx_[i]];
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
 *  \brief Add benders cut to a problem
 *
 *  \param s : subgradient of optimal slave variables
 *  \param x_cut : master separation point
 *  \param rhs : optimal slave value
 */
void WorkerMaster::add_cut(Point const &s, Point const &x_cut,
                           double const &rhs) const {
  // cut is -rhs >= overall_subpb_cost_under_approx  + s^(x-x_cut)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  DefineRhsWithMasterVariable(s, x_cut, rhs, rowrhs);
  define_matval_mclind(s, matval, mclind);

  solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::DefineRhsWithMasterVariable(
    const Point &s, const Point &x_cut, const double &rhs,
    std::vector<double> &rowrhs) const {
  rowrhs.front() -= rhs;
  for (auto const &kvp : _name_to_id) {
    if (s.find(kvp.first) != s.end()) {
      rowrhs.front() +=
          (s.find(kvp.first)->second * x_cut.find(kvp.first)->second);
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
  // cut is -rhs >= overall_subpb_cost_under_approx  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_from_sx0(sx0, rhs, rowrhs);
  define_matval_mclind(s, matval, mclind);
  solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
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
  // cut is -rhs >= overall_subpb_cost_under_approx  + s^(x-x0)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  define_rhs_from_sx0(sx0, rhs, rowrhs);
  define_matval_mclind_for_index(i, s, matval, mclind);

  solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
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
  mclind.back() = id_single_subpb_costs_under_approx_[i];
  matval.back() = -1;
}

/*!
 *  \brief Add one benders cut to a problem
 *
 *  \param i : identifier of a subproblem
 *  \param s : optimal slave variables
 *  \param x_cut : optimal Master variables
 *  \param rhs : optimal slave value
 */
// TODO : Refactor this with add_cut and define_matval_mclind(_for_index)
void WorkerMaster::addSubproblemCut(int i, Point const &s, Point const &x_cut,
                                    double const &rhs) const {
  // cut is -theta_i + s.x <= -subproblem_cost + s.x_cut (in the solver)
  // i.e. theta_i >= subproblem_cost + s.(x - x_cut) (human form)
  int ncoeffs(1 + (int)s.size());
  std::vector<char> rowtype(1, 'L');
  std::vector<double> rowrhs(1, 0);
  std::vector<double> matval(ncoeffs, 1);
  std::vector<int> mstart = {0, ncoeffs};
  std::vector<int> mclind(ncoeffs);

  DefineRhsWithMasterVariable(s, x_cut, rhs, rowrhs);
  define_matval_mclind_for_index(i, s, matval, mclind);

  solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
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

void WorkerMaster::_set_alpha_var() {
  // add the variable overall_subpb_cost_under_approx
  const std::string alpha_str("alpha");
  auto const it(_name_to_id.find(alpha_str));
  if (it == _name_to_id.end()) {
    id_single_subpb_costs_under_approx_.resize(subproblems_count, -1);

    if (_mps_has_alpha) {
      _id_alpha = _solver->get_col_index(alpha_str);
      for (int i(0); i < subproblems_count; ++i) {
        std::stringstream buffer;
        buffer << "alpha_" << i;
        id_single_subpb_costs_under_approx_[i] =
            _solver->get_col_index(buffer.str());
      }
    } else {
      double lb(-1e10); /*!< Lower Bound */
      double ub(+1e20); /*!< Upper Bound*/
      double obj(+1);
      _id_alpha =
          _solver->get_ncols(); /* Set the number of columns in _id_alpha */

      solver_addcols(
          *_solver, DblVector(1, obj), IntVector(1, 0), IntVector(0, 0),
          DblVector(0, 0.0), DblVector(1, lb), DblVector(1, ub),
          CharVector(1, 'C'),
          StrVector(1,
                    alpha_str)); /* Add variable overall_subpb_cost_under_approx
                                    and its parameters */

      for (int i(0); i < subproblems_count; ++i) {
        std::stringstream buffer;
        buffer << "alpha_" << i;
        id_single_subpb_costs_under_approx_[i] = _solver->get_ncols();
        solver_addcols(
            *_solver, DblVector(1, 0.0), IntVector(1, 0), IntVector(0, 0),
            DblVector(0, 0.0), DblVector(1, lb), DblVector(1, ub),
            CharVector(1, 'C'),
            StrVector(
                1,
                buffer.str())); /* Add variable single_subpb_costs_under_approx
                                   and its parameters */
      }

      std::vector<char> rowtype = {'E'};
      std::vector<double> rowrhs = {0};
      std::vector<int> mstart = {0, subproblems_count + 1};
      std::vector<double> matval(subproblems_count + 1, 0);
      std::vector<int> mclind(subproblems_count + 1);
      mclind[0] = _id_alpha;
      matval[0] = 1;
      for (int i(0); i < subproblems_count; ++i) {
        mclind[i + 1] = id_single_subpb_costs_under_approx_[i];
        matval[i + 1] = -1;
      }

      solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
    }
  } else {
    logger_->display_message(
        "ERROR There is already a variable called "
        "overall_subpb_cost_under_approx in the input.",
        LogUtils::LOGLEVEL::ERR, "Alpha var");
  }
}

/*!
 *  \brief Fix an upper bound and the variable overall_subpb_cost_under_approx
 * of a problem
 *
 *  \param bestUB : bound to fix
 */
void WorkerMaster::fix_alpha(double const &bestUB) const {
  std::vector<int> mindex(1, _id_alpha);
  std::vector<char> bnd_types(1, 'U');
  std::vector<double> bnd_values(1, bestUB);
  _solver->chg_bounds(mindex, bnd_types, bnd_values);
}

void WorkerMaster::_set_nb_units_var_ids() {
  int ncols = _solver->get_ncols();
  std::vector<char> col_types(ncols);
  _solver->get_col_type(col_types.data(), 0, ncols - 1);

  for (int i(0); i < col_types.size(); i++) {
    if (col_types[i] == 'I') {
      _id_nb_units.push_back(i);
    }
  }
}

void WorkerMaster::DeactivateIntegrityConstraints() const {
  std::vector<char> col_types(_id_nb_units.size(), 'C');
  _solver->chg_col_type(_id_nb_units, col_types);
}

void WorkerMaster::ActivateIntegrityConstraints() const {
  std::vector<char> col_types(_id_nb_units.size(), 'I');
  _solver->chg_col_type(_id_nb_units, col_types);
}
