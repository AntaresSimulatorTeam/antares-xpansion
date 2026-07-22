#include "SolverClp.h"

#include <atomic>

#include "COIN_common_functions.h"
using namespace std::literals;

namespace
{
std::atomic<int>& number_of_problems_counter()
{
    static std::atomic<int> counter{0};
    return counter;
}
} // namespace

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor
--------------------------------
*************************************************************************************************/

SolverClp::SolverClp(const SolverLogManager& log_manager):
    SolverClp()
{
    _fp = log_manager.log_file_ptr;
    if (_fp)
    {
        _clp.messageHandler()->setFilePointer(_fp);
    }
}

SolverClp* SolverClp::clone() const
{
    return new SolverClp(*this);
}

SolverClp::SolverClp()
{
    number_of_problems_counter() += 1;
    set_output_log_level(0);
}

SolverClp::SolverClp(const SolverClp& toCopy):
    SolverClp()
{
    _clp = ClpSimplex(toCopy._clp);
    _fp = toCopy._fp;
    if (_fp)
    {
        _clp.messageHandler()->setFilePointer(_fp);
    }
}

SolverClp::~SolverClp()
{
    number_of_problems_counter() -= 1;
    SolverClp::free();
}

int SolverClp::get_number_of_instances()
{
    return number_of_problems_counter();
}

/*************************************************************************************************
-----------------------------------    Output stream management
------------------------------
*************************************************************************************************/

/*************************************************************************************************
------------    Destruction of inner strctures and datas, closing environments
---------------
*************************************************************************************************/
void SolverClp::init()
{
    _clp = ClpSimplex();
}

void SolverClp::free()
{
    //_clp = ClpSimplex();
}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/
void SolverClp::write_prob_mps(const std::filesystem::path& filename)
{
    auto filename_to_use = filename;
    if (filename_to_use.extension() != ".mps")
    {
        filename_to_use.replace_extension(".mps");
    }
    _clp.writeMps(filename_to_use.string().c_str(), 1);
}

void SolverClp::write_prob_lp(const std::filesystem::path& filename)
{
    _clp.writeLp(filename.string().c_str());
}

void SolverClp::write_basis(const std::filesystem::path& filename)
{
    auto filename_str = filename.string();
    auto filename_c_str = filename_str.c_str();

    /*
    formatType must be = 0 (normal accuracy) to avoid undefined behavior.
    see Adr in
    conception/Architecture_decision_records/Change_Solver_Basis_Format.md
    */
    int status = _clp.writeBasis(filename_c_str, false, 0);
    zero_status_check(status, "write basis", LOGLOCATION);
}

void SolverClp::read_prob_mps(const std::filesystem::path& filename)
{
    auto filename_to_use = filename;
    if (filename_to_use.extension() != ".mps")
    {
        filename_to_use.replace_extension(".mps");
    }
    int status = _clp.readMps(filename_to_use.string().c_str(), true, false);
    zero_status_check(status, " Clp readMps "s + filename_to_use.string(), LOGLOCATION);
}

void SolverClp::read_prob_lp(const std::filesystem::path& filename)
{
    _clp.readLp(filename.string().c_str());
}

void SolverClp::read_basis(const std::filesystem::path& filename)
{
    _clp.readBasis(filename.string().c_str());
}

void SolverClp::set_basis(std::span<int> rstatus, std::span<int> cstatus)
{
    for (int i = 0; i < rstatus.size(); ++i)
    {
        _clp.setRowStatus(i, static_cast<ClpSimplex::Status>(rstatus[i]));
    }
    for (int i = 0; i < cstatus.size(); ++i)
    {
        _clp.setColumnStatus(i, static_cast<ClpSimplex::Status>(cstatus[i]));
    }
}

/*************************************************************************************************
-----------------------    Get general informations about problem
----------------------------
*************************************************************************************************/
int SolverClp::get_ncols() const
{
    return _clp.getNumCols();
}

int SolverClp::get_nrows() const
{
    return _clp.getNumRows();
}

int SolverClp::get_nelems() const
{
    return _clp.getNumElements();
}

int SolverClp::get_n_integer_vars() const
{
    int nvars = get_ncols();
    int n_int_vars(0);
    for (int i(0); i < nvars; i++)
    {
        if (_clp.isInteger(i))
        {
            n_int_vars += 1;
        }
    }
    return n_int_vars;
}

void SolverClp::get_obj(double* obj, int first, int last) const
{
    double* internalObj = _clp.objective();

    for (int i = first; i < last + 1; i++)
    {
        obj[i - first] = internalObj[i];
    }
}

void SolverClp::set_obj_to_zero()
{
    auto ncols = get_ncols();
    std::vector<double> zeros_val(ncols, 0.0);
    _clp.setRowObjective(zeros_val.data());
}

void SolverClp::set_obj(const double* obj, int first, int last)
{
    if (last - first + 1 == get_ncols())
    {
        _clp.setRowObjective(obj);
    }
    else
    {
        for (int index = first; index < last + 1; ++index)
        {
            _clp.setObjCoeff(index, obj[index]);
        }
    }
}

void SolverClp::get_rows(int* mstart,
                         int* mclind,
                         double* dmatval,
                         int size,
                         int* nels,
                         int first,
                         int last) const
{
    CoinPackedMatrix matrix = *_clp.matrix();
    matrix.reverseOrdering();
    coin_common::fill_rows_from_COIN_matrix(matrix, mstart, mclind, dmatval, nels, first, last);
}

void SolverClp::get_row_type(char* qrtype, int first, int last) const
{
    const double* rowLower = _clp.getRowLower();
    const double* rowUpper = _clp.getRowUpper();

    coin_common::fill_row_type_from_row_bounds(rowLower, rowUpper, qrtype, first, last);
}

void SolverClp::get_rhs(double* rhs, int first, int last) const
{
    const double* rowLower = _clp.getRowLower();
    const double* rowUpper = _clp.getRowUpper();
    coin_common::fill_rhs_from_bounds(rowLower, rowUpper, rhs, first, last);
}

void SolverClp::get_rhs_range(double* range, int first, int last) const
{
    auto error = LOGLOCATION + "ERROR : get rhs range not implemented for COIN CLP interface";
    throw NotImplementedFeatureSolverException(error);
}

void SolverClp::get_cols(int* mstart,
                         int* mrwind,
                         double* dmatval,
                         int size,
                         int* nels,
                         int first,
                         int last) const
{
    auto error = LOGLOCATION + "ERROR : get_cols not implemented in the CLP interface.";
    throw NotImplementedFeatureSolverException(error);
}

void SolverClp::get_col_type(char* coltype, int first, int last) const
{
    const double* colLower = _clp.getColLower();
    const double* colUpper = _clp.getColUpper();

    for (int i = first; i < last + 1; i++)
    {
        if (_clp.isInteger(i))
        {
            if (colLower[i] == 0 && colUpper[i] == 1)
            {
                coltype[i - first] = 'B';
            }
            else
            {
                coltype[i - first] = 'I';
            }
        }
        else
        {
            coltype[i - first] = 'C';
        }
    }
}

void SolverClp::get_lb(double* lb, int first, int last) const
{
    const double* colLower = _clp.getColLower();

    for (int i = first; i < last + 1; i++)
    {
        lb[i - first] = colLower[i];
    }
}

void SolverClp::get_ub(double* ub, int first, int last) const
{
    const double* colUpper = _clp.getColUpper();

    for (int i = first; i < last + 1; i++)
    {
        ub[i - first] = colUpper[i];
    }
}

int SolverClp::get_row_index(const std::string& name)
{
    const auto& col_names = get_row_names();
    const auto index = std::distance(col_names.begin(),
                                     std::find(col_names.cbegin(), col_names.cend(), name));

    if (index >= col_names.end() - col_names.begin())
    {
        return -1;
    }
    else
    {
        return index;
    }
}

int SolverClp::get_col_index(const std::string& name)
{
    const auto& col_names = get_col_names();
    const auto index = std::distance(col_names.begin(),
                                     std::find(col_names.cbegin(), col_names.cend(), name));

    if (index >= col_names.end() - col_names.begin())
    {
        return -1;
    }
    else
    {
        return index;
    }
}

std::vector<std::string> SolverClp::get_row_names(int first, int last) const
{
    std::vector<std::string> names;
    names.reserve(1 + last - first);

    for (int i = first; i < last + 1; i++)
    {
        names.push_back(_clp.getRowName(i));
    }
    return names;
}

std::vector<std::string> SolverClp::get_row_names()
{
    return *_clp.rowNames();
}

std::vector<std::string> SolverClp::get_col_names(int first, int last) const
{
    std::vector<std::string> names;
    names.reserve(1 + last - first);

    for (int i = first; i < last + 1; i++)
    {
        names.push_back(_clp.getColumnName(i));
    }
    return names;
}

std::vector<std::string> SolverClp::get_col_names()
{
    return *_clp.columnNames();
}

/*************************************************************************************************
------------------------------    Methods to modify problem
----------------------------------
*************************************************************************************************/

void SolverClp::del_rows(int first, int last)
{
    std::vector<int> mindex(last - first + 1);
    for (int i = 0; i < last - first + 1; i++)
    {
        mindex[i] = first + i;
    }
    _clp.deleteRows(last - first + 1, mindex.data());
}

void SolverClp::del_cols(int first, int last)
{
    std::vector<int> mindex(last - first + 1);
    for (int i = 0; i < last - first + 1; i++)
    {
        mindex[i] = first + i;
    }
    _clp.deleteColumns(last - first + 1, mindex.data());
}

void SolverClp::add_rows(int newrows,
                         int newnz,
                         const char* qrtype,
                         const double* rhs,
                         const double* range,
                         const int* mstart,
                         const int* mclind,
                         const double* dmatval,
                         const std::vector<std::string>& row_names)
{
    std::vector<double> rowLower(newrows);
    std::vector<double> rowUpper(newrows);
    int nrowInit = get_nrows();
    coin_common::fill_row_bounds_from_new_rows_data(rowLower, rowUpper, newrows, qrtype, rhs);

    _clp.addRows(newrows, rowLower.data(), rowUpper.data(), mstart, mclind, dmatval);
    if (row_names.size() > 0)
    {
        int nrowFinal = get_nrows();
        for (int i = nrowInit; i < nrowFinal; i++)
        {
            chg_row_name(i, row_names[i - nrowInit]);
        }
    }
}

void SolverClp::add_cols(int newcol,
                         int newnz,
                         const double* objx,
                         const int* mstart,
                         const int* mrwind,
                         const double* dmatval,
                         const double* bdl,
                         const double* bdu,
                         const std::vector<std::string>& col_names)
{
    std::vector<int> colStart(newcol + 1);
    for (int i(0); i < newcol; i++)
    {
        colStart[i] = mstart[i];
    }
    colStart[newcol] = newnz;
    int ncolInit = get_ncols();

    _clp.addColumns(newcol, bdl, bdu, objx, colStart.data(), mrwind, dmatval);
    if (col_names.size() > 0)
    {
        int ncolFinal = get_ncols();
        for (int i = ncolInit; i < ncolFinal; i++)
        {
            chg_col_name(i, col_names[i - ncolInit]);
        }
    }
}

void SolverClp::add_name(int type, const char* cnames, int indice)
{
    auto error = LOGLOCATION + "ERROR : addnames not implemented in the CLP interface.";
    throw NotImplementedFeatureSolverException(error);
}

void SolverClp::add_names(int type, const std::vector<std::string>& cnames, int first, int end)
{
    // TODO
    auto error = LOGLOCATION + "ERROR : addnames not implemented in the CLP interface.";
    throw NotImplementedFeatureSolverException(error);
}

void SolverClp::chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj)
{
    assert(obj.size() == mindex.size());
    for (int i(0); i < obj.size(); i++)
    {
        _clp.setObjCoeff(mindex[i], obj[i]);
    }
}

void SolverClp::chg_obj_direction(const bool minimize)
{
    int objsense = minimize ? 1 : -1;
    _clp.setOptimizationDirection(objsense);
}

void SolverClp::chg_bounds(const std::vector<int>& mindex,
                           const std::vector<char>& qbtype,
                           const std::vector<double>& bnd)
{
    assert(qbtype.size() == mindex.size());
    assert(bnd.size() == mindex.size());
    for (int i(0); i < mindex.size(); i++)
    {
        if (qbtype[i] == 'L')
        {
            _clp.setColLower(mindex[i], bnd[i]);
        }
        else if (qbtype[i] == 'U')
        {
            _clp.setColUpper(mindex[i], bnd[i]);
        }
        else if (qbtype[i] == 'B')
        {
            _clp.setColLower(mindex[i], bnd[i]);
            _clp.setColUpper(mindex[i], bnd[i]);
        }
        else
        {
            throw InvalidBoundTypeException(qbtype[i], LOGLOCATION);
        }
    }
}

void SolverClp::chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype)
{
    assert(qctype.size() == mindex.size());
    std::vector<int> bnd_index(1, 0);
    std::vector<char> bnd_type(1, 'U');
    std::vector<double> bnd_val(1, 1.0);

    for (int i = 0; i < mindex.size(); i++)
    {
        switch (qctype[i])
        {
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
            throw InvalidColTypeException(qctype[i], LOGLOCATION);
        }
    }
}

void SolverClp::chg_rhs(int id_row, double val)
{
    const double* rowLower = _clp.getRowLower();
    const double* rowUpper = _clp.getRowUpper();

    char row_type;
    get_row_type(&row_type, id_row, id_row);

    if (row_type == 'E')
    {
        _clp.setRowLower(id_row, val);
        _clp.setRowUpper(id_row, val);
        return;
    }

    if (rowLower[id_row] <= -COIN_DBL_MAX)
    {
        if (rowUpper[id_row] >= COIN_DBL_MAX)
        {
            std::stringstream buffer;
            buffer << LOGLOCATION << "ERROR : unconstrained constraint " << id_row
                   << " in chg_rhs.";
            throw GenericSolverException(buffer.str());
        }
        else
        {
            _clp.setRowUpper(id_row, val);
        }
    }
    else
    {
        if (rowUpper[id_row] >= COIN_DBL_MAX)
        {
            _clp.setRowLower(id_row, val);
        }
        else
        {
            std::stringstream buffer;
            buffer << LOGLOCATION << "ERROR : constraint " << id_row
                   << " has both different lower and upper bound in chg_rhs.";
            throw GenericSolverException(buffer.str());
        }
    }
}

void SolverClp::chg_coef(int id_row, int id_col, double val)
{
    CoinPackedMatrix* matrix = _clp.matrix();
    matrix->modifyCoefficient(id_row, id_col, val);
}

void SolverClp::chg_coefs(const std::vector<int>& id_rows,
                          const std::vector<int>& id_cols,
                          const std::vector<double>& vals)
{
    for (size_t i = 0; i < id_rows.size(); i++)
    {
        chg_coef(id_rows[i], id_cols[i], vals[i]);
    }
}

void SolverClp::chg_rhs_values(std::vector<int>& rhs_indices, std::vector<double>& rhs_values)
{
    for (int i = 0; i < rhs_indices.size(); ++i)
    {
        chg_rhs(rhs_indices[i], rhs_values[i]);
    }
}

void SolverClp::chg_row_name(int id_row, const std::string& name)
{
    std::string copy_name = name;
    _clp.setRowName(id_row, copy_name);
}

void SolverClp::chg_col_name(int id_col, const std::string& name)
{
    std::string copy_name = name;
    _clp.setColumnName(id_col, copy_name);
}

/*************************************************************************************************
-----------------------------    Methods to solve the problem
---------------------------------
*************************************************************************************************/
int SolverClp::solve_lp()
{
    int lp_status;
    _clp.dual();

    int clp_status = _clp.status();
    if (clp_status == CLP_OPTIMAL)
    {
        lp_status = OPTIMAL;
    }
    else if (clp_status == CLP_PRIMAL_INFEASIBLE)
    {
        lp_status = INFEASIBLE;
    }
    else if (clp_status == CLP_DUAL_INFEASIBLE)
    {
        lp_status = UNBOUNDED;
    }
    else
    {
        lp_status = UNKNOWN;
        std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
    }
    return lp_status;
}

int SolverClp::solve_mip()
{
    int lp_status;
    _clp.dual();

    int clp_status = _clp.status();
    if (clp_status == CLP_OPTIMAL)
    {
        lp_status = OPTIMAL;
    }
    else if (clp_status == CLP_PRIMAL_INFEASIBLE)
    {
        lp_status = INFEASIBLE;
    }
    else if (clp_status == CLP_DUAL_INFEASIBLE)
    {
        lp_status = UNBOUNDED;
    }
    else
    {
        lp_status = UNKNOWN;
        std::cout << "Error : UNKNOWN CLP STATUS IS : " << clp_status << std::endl;
    }
    return lp_status;
}

/*************************************************************************************************
-------------------------    Methods to get solutions information
-----------------------------
*************************************************************************************************/
void SolverClp::get_basis(int* rstatus, int* cstatus) const
{
    int ncols = get_ncols();
    for (int i = 0; i < ncols; i++)
    {
        cstatus[i] = _clp.getColumnStatus(i);
    }

    int nrows = get_nrows();
    for (int i = 0; i < nrows; i++)
    {
        rstatus[i] = _clp.getRowStatus(i);
    }
}

double SolverClp::get_mip_value() const
{
    return _clp.objectiveValue();
}

double SolverClp::get_lp_value() const
{
    return _clp.objectiveValue();
}

int SolverClp::get_splex_num_of_ite_last() const
{
    return _clp.numberIterations();
}

void SolverClp::get_lp_sol(double* primals, double* duals, double* reduced_costs) const
{
    if (primals != NULL)
    {
        double* primalSol = _clp.primalColumnSolution();
        for (int i(0); i < get_ncols(); i++)
        {
            primals[i] = primalSol[i];
        }
    }

    if (duals != NULL)
    {
        double* dualSol = _clp.dualRowSolution();
        for (int i(0); i < get_nrows(); i++)
        {
            duals[i] = dualSol[i];
        }
    }

    if (reduced_costs != NULL)
    {
        double* reducedCostSolution = _clp.dualColumnSolution();
        for (int i(0); i < get_ncols(); i++)
        {
            reduced_costs[i] = reducedCostSolution[i];
            // std::cout << "RD    " << i << " " << reducedCostSolution[i] <<
            // std::endl;
        }
    }
}

void SolverClp::get_mip_sol(double* primals)
{
    if (primals != NULL)
    {
        double* primalSol = _clp.primalColumnSolution();
        for (int i(0); i < get_ncols(); i++)
        {
            primals[i] = primalSol[i];
        }
    }
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels
---------------------------
*************************************************************************************************/
void SolverClp::set_output_log_level(int loglevel)
{
    if (loglevel > 1 && _fp)
    {
        _clp.messageHandler()->setLogLevel(1);
    }
    else
    {
        _clp.messageHandler()->setLogLevel(0);
    }
}

void SolverClp::set_algorithm(const std::string& algo)
{
    if (algo == "DUAL")
    {
        _clp.setAlgorithm(1);
    }
    else
    {
        throw InvalidSolverOptionException("set_algorithm : " + algo, LOGLOCATION);
    }
}

void SolverClp::set_threads(int n_threads)
{
    _clp.setNumberThreads(n_threads);
}

void SolverClp::set_optimality_gap(double gap)
{
    throw InvalidSolverOptionException("set_optimality_gap : " + std::to_string(gap), LOGLOCATION);
}

void SolverClp::set_simplex_iter(int iter)
{
    _clp.setMaximumIterations(iter);
}

/**
 * CLP default to write_prob_mps implementation
 * @param filename
 */
void SolverClp::save_prob(const std::filesystem::path& filename)
{
    write_prob_mps(filename);
}

/**
 * CLP default to read_prob_mps implementation
 * @param filename
 */
void SolverClp::restore_prob(const std::filesystem::path& filename)
{
    read_prob_mps(filename);
}

void SolverClp::mark_indices_to_keep_presolve(int, int, int*, int*)
{
    throw NotImplementedFeatureSolverException(
      "mark_indices_to_keep_presolve is not supported for CLP solver");
}

void SolverClp::presolve_only()
{
    throw NotImplementedFeatureSolverException("presolve_only is not supported for CLP solver");
}

void SolverClp::get_presolve_map(int*, int*) const
{
    throw NotImplementedFeatureSolverException("get_presolve_map is not supported for CLP solver");
}
