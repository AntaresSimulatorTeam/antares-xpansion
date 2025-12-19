#include "antares-xpansion/benders/benders_core/WorkerMaster.h"
#include <boost/tokenizer.hpp>
#include "antares-xpansion/helpers/solver_utils.h"

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
WorkerMaster::WorkerMaster(const VariableMap& variable_map,
                           const std::string& solver_name,
                           const int log_level,
                           int subproblems_count,
                           SolverLogManager& solver_log_manager,
                           const bool mps_has_alpha,
                           Logger logger,
                           ProblemsFormat format,
                           IBendersProblemProvider* benders_problem_provider,
                           double master_solution_tolerance,
                           double cut_coefficient_tolerance):
    Worker(variable_map, std::move(logger), cut_coefficient_tolerance),
    subproblems_count{subproblems_count},
    _mps_has_alpha{mps_has_alpha},
    _master_solution_tolerance{master_solution_tolerance}
{
    _is_master = true;

    init(solver_name, log_level, solver_log_manager, format, benders_problem_provider);
    if (!_mps_has_alpha)
    {
        _set_upper_bounds();
    }
    _set_alpha_var();
    _set_nb_units_var_ids();

    std::ifstream investment_dict_path ("./investment_dictionnary.csv") ; 
   
    if (investment_dict_path.is_open()) 
    {
        std::string row ; 
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(investment_dict_path,row)) 
        {
            Tokenizer tok(row) ; 
            std::vector<std::string> tokens(tok.begin(), tok.end());
            binary_variables_ids_map_[tokens[1]] = tokens[0] ;
        }
    }

    std::cout<<"binary_variables_ids_map_ size "<<binary_variables_ids_map_.size()<<std::endl ; 
}

/*!
 *  \brief Restores feasibility of the solution returned by master in case it is not due to the
 * numerical tolerance of the solver. We do not have direct access of the feasibility tolerance of
 * the solver, hence we use a user-defined tolerance (default to 1e-4) which may be greater than the
 * default one of the solver (generally 1e-6). This is ok as we anyway want to round solutions that
 * will be sent to the subproblems back to the bounds if we are close to it.
 *
 *  \param solution : solution vector of the master problem as returned by the solver
 */
void WorkerMaster::restoreFeasibility(std::vector<double>& solution)
{
    // _id_alpha is equal to the number of variable already present in the problem before alpha vars
    // are added
    std::vector<char> col_type(_id_alpha);
    std::vector<double> lb(_id_alpha);
    std::vector<double> ub(_id_alpha);

    solver_getcolinfo(*_solver, col_type, lb, ub, 0, _id_alpha - 1);
    for (const auto var_id: _id_to_name | std::views::keys)
    {
        double value = solution[var_id];
        // Case variable slightly above ub
        if (value > ub[var_id] && value < ub[var_id] + _master_solution_tolerance)
        {
            solution[var_id] = ub[var_id];
        }
        // Case variable slightly lower than lb
        else if (value < lb[var_id] && value > lb[var_id] - _master_solution_tolerance)
        {
            solution[var_id] = lb[var_id];
        }
        // Case integer variable
        else if (col_type[var_id] == 'B' || col_type[var_id] == 'I')
        {
            int rounded = std::round(value);
            solution[var_id] = std::abs(value - rounded) < _master_solution_tolerance ? rounded
                                                                                      : value;
        }
    }
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
void WorkerMaster::get(Point& x_out,
                       double& overall_subpb_cost_under_approx,
                       DblVector& single_subpb_costs_under_approx)
{
    x_out.clear();
    std::vector<double> solution(_solver->get_ncols());

    if (_solver->get_n_integer_vars() > 0)
    {
        _solver->get_mip_sol(solution.data());
    }
    else
    {
        _solver->get_lp_sol(solution.data(), nullptr, nullptr);
    }
    assert(_id_single_subpb_costs_under_approx.back() + 1 == solution.size());
    restoreFeasibility(solution);
    for (const auto& kvp: _id_to_name)
    {
        x_out[kvp.second] = solution[kvp.first];
    }
    overall_subpb_cost_under_approx = solution[_id_alpha];
    for (int i(0); i < _id_single_subpb_costs_under_approx.size(); ++i)
    {
        single_subpb_costs_under_approx[i] = solution[_id_single_subpb_costs_under_approx[i]];
    }
}

/*!
 *  \brief Set dual values of a problem in a vector
 *
 *  \param dual : reference to a vector of double
 */
void WorkerMaster::get_dual_values(std::vector<double>& dual) const
{
    dual.resize(get_number_constraint());
    solver_getlpdual(_solver, dual);
}

/*!
 *  \brief Return number of constraint in a problem
 */
int WorkerMaster::get_number_constraint() const
{
    return _solver->get_nrows();
}

/*!
 *  \brief Add benders cut to a problem
 *
 *  \param s : subgradient of optimal slave variables
 *  \param x_cut : master separation point
 *  \param rhs : optimal slave value
 */
void WorkerMaster::add_cut(const Point& s, const Point& x_cut, const double& rhs) const
{
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

void WorkerMaster::DefineRhsWithMasterVariable(const Point& s,
                                               const Point& x_cut,
                                               const double& rhs,
                                               std::vector<double>& rowrhs) const
{
    rowrhs.front() -= rhs;
    for (const auto& kvp: _name_to_id)
    {
        if (s.find(kvp.first) != s.end())
        {
            rowrhs.front() += (s.find(kvp.first)->second * x_cut.find(kvp.first)->second);
        }
    }
}

void WorkerMaster::define_matval_mclind(const Point& s,
                                        std::vector<double>& matval,
                                        std::vector<int>& mclind) const
{
    size_t mclindCnt_l(0);
    for (const auto& kvp: _name_to_id)
    {
        if (s.find(kvp.first) != s.end())
        {
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
void WorkerMaster::add_dynamic_cut(const Point& s, const double& sx0, const double& rhs) const
{
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

void WorkerMaster::define_rhs_from_sx0(const double& sx0,
                                       const double& rhs,
                                       std::vector<double>& rowrhs) const
{
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
void WorkerMaster::add_cut_by_iter(const int i,
                                   const Point& s,
                                   const double& sx0,
                                   const double& rhs) const
{
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

void WorkerMaster::define_matval_mclind_for_index(const int i,
                                                  const Point& s,
                                                  std::vector<double>& matval,
                                                  std::vector<int>& mclind) const
{
    size_t mclindCnt_l(0);
    for (const auto& kvp: _name_to_id)
    {
        if (s.find(kvp.first) != s.end())
        {
            mclind[mclindCnt_l] = kvp.second;
            matval[mclindCnt_l] = s.find(kvp.first)->second;
            ++mclindCnt_l;
        }
    }
    mclind.back() = _id_single_subpb_costs_under_approx[i];
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
void WorkerMaster::addSubproblemCut(int i,
                                    const Point& subgradient,
                                    const Point& x_cut,
                                    const double& rhs) const
{
    // cut is -theta_i + subgradient.x <= -subproblem_cost + subgradient.x_cut (in the solver)
    // i.e. theta_i >= subproblem_cost + subgradient.(x - x_cut) (human form)
    int ncoeffs(1 + (int)subgradient.size());
    std::vector<char> rowtype(1, 'L');
    std::vector<double> rowrhs(1, 0);
    std::vector<double> matval(ncoeffs, 1);
    std::vector<int> mstart = {0, ncoeffs};
    std::vector<int> mclind(ncoeffs);

    DefineRhsWithMasterVariable(subgradient, x_cut, rhs, rowrhs);
    define_matval_mclind_for_index(i, subgradient, matval, mclind);

    // Round numerically small rhs to zero to get clean cuts and avoid numerical artifacts
    // Cuts coefficients (obtained from subgradient) have already been rounded in
    // SubproblemWorker::get_subgradient as it is best to round it as soon as possible (because
    // subgradient information is also used as is to compute cut values : cf. compute_cut_val())
    roundIfWithinTolerance(rowrhs, 0, rowrhs.size());

    solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
}

void WorkerMaster::_set_upper_bounds() const
{
    // Cbc solver sets infinite upper bounds to DBL_MAX = 1.79769e+308 which is
    // way too large as it appears in datas.max_invest. We set it to 1e20
    int ncols = _solver->get_ncols();
    DblVector bounds(ncols);
    _solver->get_ub(bounds.data(), 0, ncols - 1);
    CharVector bndTypes(ncols, 'U');
    IntVector indices(ncols);
    for (int i = 0; i < _solver->get_ncols(); i++)
    {
        indices[i] = i;
        bounds[i] = std::min(bounds[i], 1e20);
    }
    _solver->chg_bounds(indices, bndTypes, bounds);
}

void WorkerMaster::_set_alpha_var()
{
    // add the variable overall_subpb_cost_under_approx
    const std::string alpha_str("alpha");
    const auto it(_name_to_id.find(alpha_str));
    if (it == _name_to_id.end())
    {
        _id_single_subpb_costs_under_approx.resize(subproblems_count, -1);

        if (_mps_has_alpha)
        {
            _id_alpha = _solver->get_col_index(alpha_str);
            for (int i(0); i < subproblems_count; ++i)
            {
                std::stringstream buffer;
                buffer << "alpha_" << i;
                _id_single_subpb_costs_under_approx[i] = _solver->get_col_index(buffer.str());
            }
        }
        else
        {
            double lb(-1e10); /*!< Lower Bound */
            double ub(+1e20); /*!< Upper Bound*/
            double obj(+1);
            _id_alpha = _solver->get_ncols(); /* Set the number of columns in _id_alpha */

            solver_addcols(*_solver,
                           DblVector(1, obj),
                           IntVector(1, 0),
                           IntVector(0, 0),
                           DblVector(0, 0.0),
                           DblVector(1, lb),
                           DblVector(1, ub),
                           CharVector(1, 'C'),
                           StrVector(1, alpha_str)); /* Add variable overall_subpb_cost_under_approx
                                                        and its parameters */

            for (int i(0); i < subproblems_count; ++i)
            {
                std::stringstream buffer;
                buffer << "alpha_" << i;
                _id_single_subpb_costs_under_approx[i] = _solver->get_ncols();
                solver_addcols(
                  *_solver,
                  DblVector(1, 0.0),
                  IntVector(1, 0),
                  IntVector(0, 0),
                  DblVector(0, 0.0),
                  DblVector(1, lb),
                  DblVector(1, ub),
                  CharVector(1, 'C'),
                  StrVector(1, buffer.str())); /* Add variable single_subpb_costs_under_approx
                                                  and its parameters */
            }

            std::vector<char> rowtype = {'E'};
            std::vector<double> rowrhs = {0};
            std::vector<int> mstart = {0, subproblems_count + 1};
            std::vector<double> matval(subproblems_count + 1, 0);
            std::vector<int> mclind(subproblems_count + 1);
            mclind[0] = _id_alpha;
            matval[0] = 1;
            for (int i(0); i < subproblems_count; ++i)
            {
                mclind[i + 1] = _id_single_subpb_costs_under_approx[i];
                matval[i + 1] = -1;
            }

            solver_addrows(*_solver, rowtype, rowrhs, {}, mstart, mclind, matval);
        }
    }
    else
    {
        logger_->display_message("ERROR There is already a variable called "
                                 "overall_subpb_cost_under_approx in the input.",
                                 LogUtils::LOGLEVEL::ERR,
                                 "Alpha var");
    }
}

/*!
 *  \brief Fix an upper bound and the variable overall_subpb_cost_under_approx
 * of a problem
 *
 *  \param bestUB : bound to fix
 */
void WorkerMaster::fix_alpha(const double& bestUB) const
{
    std::vector<int> mindex(1, _id_alpha);
    std::vector<char> bnd_types(1, 'U');
    std::vector<double> bnd_values(1, bestUB);
    _solver->chg_bounds(mindex, bnd_types, bnd_values);
}

void WorkerMaster::_set_nb_units_var_ids()
{
    int ncols = _solver->get_ncols();
    std::vector<char> col_types(ncols);
    _solver->get_col_type(col_types.data(), 0, ncols - 1);

    for (int i(0); i < col_types.size(); i++)
    {
        if (col_types[i] == 'I' || col_types[i] == 'B')
        {
            _id_int_vars.push_back(i);
        }
    }
}

void WorkerMaster::DeactivateIntegrityConstraints() const
{
    std::vector<char> col_types(_id_int_vars.size(), 'C');
    _solver->chg_col_type(_id_int_vars, col_types);
}

void WorkerMaster::ActivateIntegrityConstraints() const
{
    std::vector<char> col_types(_id_int_vars.size(), 'I');
    _solver->chg_col_type(_id_int_vars, col_types);
}


void WorkerMaster::write_main_variable_csv(std::filesystem::path& main_variables_csv_path, const Point& x_out) 
{
    std::ofstream csv_file(main_variables_csv_path) ; 
    if (csv_file.is_open()) 
    {
        for (const auto& [id,value] : x_out) 
        {
            csv_file<<binary_variables_ids_map_[id]<<","<<value<<"\n"; 
        }
    }
    csv_file.close() ; 
}

