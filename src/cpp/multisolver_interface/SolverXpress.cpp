#include "antares-xpansion/multisolver_interface/SolverXpress.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <map>
#include <numeric>
#include <set>
#include <utility>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

using namespace LoadXpress;
using namespace std::literals;
/*************************************************************************************************
-----------------------------------    Constructor/Desctructor
--------------------------------
*************************************************************************************************/
const std::map<int, std::string> TYPETONAME = {{1, "rows"}, {2, "columns"}};

namespace
{
std::atomic<int>& number_of_problems_counter()
{
    static std::atomic<int> counter{0};
    return counter;
}
} // namespace

SolverXpress::SolverXpress(const SolverLogManager& log_manager):
    SolverXpress()
{
    if (log_manager.log_file_path != "")
    {
        _log_file = log_manager.log_file_path;
        _log_stream.open(_log_file, std::ofstream::out | std::ofstream::app);
        add_stream(_log_stream);
    }
}

SolverXpress* SolverXpress::clone() const
{
    return new SolverXpress(*this);
}

namespace
{
std::mutex& instance_guard()
{
    static std::mutex license_guard;
    return license_guard;
}
} // namespace

XpressManager::XpressManager(std::shared_ptr<ILoggerXpansion> logger):
    logger_(logger),
    loader_(logger)
{
    std::lock_guard guard(instance_guard());
    if (loader_.XpressIsCorrectlyInstalled())
    {
        int status = XPRSinit(nullptr);
        SolverAbstract::zero_status_check(status, "initialize XPRESS environment", LOGLOCATION);
    }
}

XpressManager::~XpressManager()
{
    std::lock_guard guard(instance_guard());

    if (loader_.XpressIsCorrectlyInstalled())
    {
        if (int status = XPRSfree())
        {
            std::cerr << "Failed to free XPRESS environment with status: " << status << " "
                      << LOGLOCATION << std::endl;
        }
    }
}

SolverXpress::SolverXpress()
{
    number_of_problems_counter() += 1;
    _xprs = nullptr;
}

SolverXpress::SolverXpress(const SolverXpress& toCopy):
    SolverXpress()
{
    XpressLoader loader;
    loader.XpressIsCorrectlyInstalled();

    _xprs = toCopy.clone_matrix_to_new_prob();
    _log_file = toCopy._log_file;
    if (_log_file != "")
    {
        _log_stream.open(_log_file, std::ofstream::out | std::ofstream::app);
        add_stream(_log_stream);
    }
}

SolverXpress::~SolverXpress()
{
    number_of_problems_counter() -= 1;
    SolverXpress::free();

    if (_log_stream.is_open())
    {
        _log_stream.close();
    }
}

int SolverXpress::get_number_of_instances()
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
void SolverXpress::init()
{
    int status = XPRScreateprob(&_xprs);
    zero_status_check(status, "create XPRESS problem", LOGLOCATION);

    SolverXpress::set_output_log_level(0);
    status = XPRSloadlp(_xprs,
                        "empty",
                        0,
                        0,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr);
    zero_status_check(status, "generate empty prob in XPRS interface init method", LOGLOCATION);
}

void SolverXpress::free()
{
    if (_xprs)
    {
        int status = XPRSdestroyprob(_xprs);
        _xprs = nullptr;
        if (status)
        {
            for (auto& stream: get_stream())
            {
                *stream << "Failed to destroy XPRESS problem with status: " << status << " "
                        << LOGLOCATION;
            }
        }
    }
}

/*************************************************************************************************
-------------------------------    Reading & Writing problems
-------------------------------
*************************************************************************************************/
void SolverXpress::write_prob_mps(const std::filesystem::path& filename)
{
    std::string nFlags = "";
    int status = XPRSwriteprob(_xprs, filename.string().c_str(), nFlags.c_str());
    zero_status_check(status, "write problem", LOGLOCATION);
}

void SolverXpress::write_prob_lp(const std::filesystem::path& filename)
{
    std::string nFlags = "l";
    int status = XPRSwriteprob(_xprs, filename.string().c_str(), nFlags.c_str());
    zero_status_check(status, "write problem", LOGLOCATION);
}

const std::string WRITE_SOL_VALUES = "n";

void SolverXpress::write_basis(const std::filesystem::path& filename)
{
    int status = XPRSwritebasis(_xprs, filename.string().c_str(), WRITE_SOL_VALUES.c_str());
    zero_status_check(status, "write basis", LOGLOCATION);
}

void SolverXpress::read_prob_mps(const std::filesystem::path& filename)
{
    std::string nFlags = "";
    read_prob(filename.string().c_str(), nFlags.c_str());
}

void SolverXpress::read_prob_lp(const std::filesystem::path& filename)
{
    std::string nFlags = "l";
    read_prob(filename.string().c_str(), nFlags.c_str());
}

void SolverXpress::read_prob(const char* prob_name, const char* flags)
{
    // To delete obj from rows when reading prob
    int keeprows(0);
    int status = XPRSgetintcontrol(_xprs, XPRS_KEEPNROWS, &keeprows);
    zero_status_check(status, "get XPRS_KEEPNROWS", LOGLOCATION);

    /*
     * This part of the code requires XPRESS version 8.8.5 or higher
     * For previous versions, the param KEEPNROWS deletes row names
     * when reading.
     * In order to bypass that without modifying the code,
     * the first row, which is the objective function
     * is deleted after read
     *
     */

    /*
    if (keeprows != -1) {
            status = XPRSsetintcontrol(_xprs, XPRS_KEEPNROWS, -1);
            zero_status_check(status, "set XPRS_KEEPNROWS to -1",LOGLOCATION);
    }
    */

    status = XPRSreadprob(_xprs, prob_name, flags);
    zero_status_check(status, " read problem "s + prob_name, LOGLOCATION);

    // If param KEEPNROWS not -1 remove first row which is the objective function
    if (keeprows != -1)
    {
        del_rows(0, 0);
    }
}

void SolverXpress::read_basis(const std::filesystem::path& filename)
{
    int status = XPRSreadbasis(_xprs, filename.string().c_str(), "");
    zero_status_check(status, "read basis", LOGLOCATION);
}

void SolverXpress::set_basis(std::span<int> rstatus, std::span<int> cstatus)
{
    int status = XPRSloadbasis(_xprs, rstatus.data(), cstatus.data());
    zero_status_check(status, "set basis", LOGLOCATION);
}

/*************************************************************************************************
-----------------------    Get general informations about problem
----------------------------
*************************************************************************************************/
int SolverXpress::get_ncols() const
{
    int cols(0);
    int status = XPRSgetintattrib(_xprs, XPRS_COLS, &cols);
    zero_status_check(status, "get number of columns", LOGLOCATION);
    return cols;
}

int SolverXpress::get_nrows() const
{
    int rows(0);
    int status = XPRSgetintattrib(_xprs, XPRS_ROWS, &rows);
    zero_status_check(status, "get number of rows", LOGLOCATION);
    return rows;
}

int SolverXpress::get_nelems() const
{
    int elems(0);
    int status = XPRSgetintattrib(_xprs, XPRS_ELEMS, &elems);
    zero_status_check(status, "get number of non zero elements", LOGLOCATION);

    return elems;
}

int SolverXpress::get_n_integer_vars() const
{
    int n_int_vars(0);
    int status = XPRSgetintattrib(_xprs, XPRS_MIPENTS, &n_int_vars);
    zero_status_check(status, "get number of integer variables", LOGLOCATION);
    return n_int_vars;
}

void SolverXpress::get_obj(double* obj, int first, int last) const
{
    int status = XPRSgetobj(_xprs, obj, first, last);
    zero_status_check(status, "get objective function", LOGLOCATION);
}

void SolverXpress::set_obj_to_zero()
{
    auto ncols = get_ncols();
    std::vector<double> zeros_val(ncols, 0.0);
    set_obj(zeros_val.data(), 0, ncols);
}

void SolverXpress::set_obj(const double* obj, int first, int last)
{
    auto ncols = last - first + 1;
    std::vector<int> col_ind(ncols);
    std::iota(col_ind.begin(), col_ind.end(), first);
    int status = XPRSchgobj(_xprs, ncols, col_ind.data(), obj);
    zero_status_check(status, "set objective function", LOGLOCATION);
}

void SolverXpress::get_rows(int* mstart,
                            int* mclind,
                            double* dmatval,
                            int size,
                            int* nels,
                            int first,
                            int last) const
{
    int status = XPRSgetrows(_xprs, mstart, mclind, dmatval, size, nels, first, last);
    zero_status_check(status, "get rows", LOGLOCATION);
}

void SolverXpress::get_row_type(char* qrtype, int first, int last) const
{
    int status = XPRSgetrowtype(_xprs, qrtype, first, last);
    zero_status_check(status, "get rows types", LOGLOCATION);
}

void SolverXpress::get_rhs(double* rhs, int first, int last) const
{
    int status = XPRSgetrhs(_xprs, rhs, first, last);
    zero_status_check(status, "get RHS", LOGLOCATION);
}

void SolverXpress::get_rhs_range(double* range, int first, int last) const
{
    int status = XPRSgetrhsrange(_xprs, range, first, last);
    zero_status_check(status, "get RHS of range rows", LOGLOCATION);
}

void SolverXpress::get_cols(int* mstart,
                            int* mrwind,
                            double* dmatval,
                            int size,
                            int* nels,
                            int first,
                            int last) const
{
    int status = XPRSgetcols(_xprs, mstart, mrwind, dmatval, size, nels, first, last);
    zero_status_check(status, "get cols", LOGLOCATION);
}

void SolverXpress::get_col_type(char* coltype, int first, int last) const
{
    int status = XPRSgetcoltype(_xprs, coltype, first, last);
    zero_status_check(status, "get type of columns", LOGLOCATION);
}

void SolverXpress::get_lb(double* lb, int first, int last) const
{
    int status = XPRSgetlb(_xprs, lb, first, last);
    zero_status_check(status, "get lower bounds of variables", LOGLOCATION);
}

void SolverXpress::get_ub(double* ub, int first, int last) const
{
    int status = XPRSgetub(_xprs, ub, first, last);
    zero_status_check(status, "get upper bounds of variables", LOGLOCATION);
}

int SolverXpress::get_row_index(const std::string& name)
{
    int id = 0;
    int status = XPRSgetindex(_xprs, 1, name.c_str(), &id);
    zero_status_check(status, "get row index. Name does not exist.", LOGLOCATION);
    return id;
}

int SolverXpress::get_col_index(const std::string& name)
{
    int id = 0;
    int status = XPRSgetindex(_xprs, 2, name.c_str(), &id);
    zero_status_check(status, "get column index. Name does not exist.", LOGLOCATION);
    return id;
}

std::vector<std::string> SolverXpress::get_names(int type, int n_elements) const
{
    return get_names(type, 0, n_elements - 1);
}

std::vector<std::string> SolverXpress::get_names(int type, int first, int last) const
{
    int xprs_name_length;
    zero_status_check(XPRSgetintattrib(_xprs, XPRS_NAMELENGTH, &xprs_name_length),
                      "get xprs name length",
                      LOGLOCATION);

    std::string names_in_one_string;
    names_in_one_string.resize((8 * xprs_name_length + 1) * (last - first + 1));

    zero_status_check(XPRSgetnames(_xprs, type, names_in_one_string.data(), first, last),
                      "get " + TYPETONAME.at(type) + " names.",
                      LOGLOCATION);
    names_in_one_string = StringManip::trim_in_place(names_in_one_string);
    auto res = StringManip::split(names_in_one_string, '\0');
    return res;
}

namespace
{
enum XPRESS_NAMES_TYPE : int
{
    ROW = 1,
    COLUMN = 2
};
} // namespace

std::vector<std::string> SolverXpress::get_row_names(int first, int last) const
{
    return get_names(XPRESS_NAMES_TYPE::ROW, first, last);
}

std::vector<std::string> SolverXpress::get_row_names()
{
    return get_names(XPRESS_NAMES_TYPE::ROW, get_nrows());
}

std::vector<std::string> SolverXpress::get_col_names(int first, int last) const
{
    return get_names(XPRESS_NAMES_TYPE::COLUMN, first, last);
}

std::vector<std::string> SolverXpress::get_col_names()
{
    return get_names(XPRESS_NAMES_TYPE::COLUMN, get_ncols());
}

/*************************************************************************************************
------------------------------    Methods to modify problem
----------------------------------
*************************************************************************************************/

void SolverXpress::del_rows(int first, int last)
{
    std::vector<int> mindex(last - first + 1);
    for (int i = 0; i < last - first + 1; i++)
    {
        mindex[i] = first + i;
    }
    int status = XPRSdelrows(_xprs, last - first + 1, mindex.data());
    zero_status_check(status, "delete rows", LOGLOCATION);
}

void SolverXpress::del_cols(int first, int last)
{
    std::vector<int> mindex(last - first + 1);
    for (int i = 0; i < last - first + 1; i++)
    {
        mindex[i] = first + i;
    }
    int status = XPRSdelcols(_xprs, last - first + 1, mindex.data());
    zero_status_check(status, "delete columns", LOGLOCATION);
}

void SolverXpress::add_rows(int newrows,
                            int newnz,
                            const char* qrtype,
                            const double* rhs,
                            const double* range,
                            const int* mstart,
                            const int* mclind,
                            const double* dmatval,
                            const std::vector<std::string>& row_names)
{
    int nrowInit = get_nrows();
    int status = XPRSaddrows(_xprs, newrows, newnz, qrtype, rhs, range, mstart, mclind, dmatval);
    zero_status_check(status, "add rows", LOGLOCATION);
    if (row_names.size() > 0)
    {
        int nrowFinal = get_nrows();
        add_names(1, row_names, nrowInit, nrowFinal - 1);
    }
}

void SolverXpress::add_cols(int newcol,
                            int newnz,
                            const double* objx,
                            const int* mstart,
                            const int* mrwind,
                            const double* dmatval,
                            const double* bdl,
                            const double* bdu,
                            const std::vector<std::string>& col_names)
{
    int ncolInit = get_ncols();
    int status = XPRSaddcols(_xprs, newcol, newnz, objx, mstart, mrwind, dmatval, bdl, bdu);
    zero_status_check(status, "add columns", LOGLOCATION);
    if (col_names.size() > 0)
    {
        int ncolFinal = get_ncols();
        add_names(2, col_names, ncolInit, ncolFinal - 1);
    }
}

void SolverXpress::add_name(int type, const char* cnames, int indice)
{
    int status = XPRSaddnames(_xprs, type, cnames, indice, indice);
    zero_status_check(status, "add names", LOGLOCATION);
}

namespace
{
int add_names(XPRSprob prob, int type, const std::vector<std::string>& cnames, int first, int end)
{
    auto names = std::make_unique<char[]>(
      /* +1 for final '\0' */ std::accumulate(cnames.begin(),
                                              cnames.end(),
                                              1,
                                              [](int sum, const std::string& s)
                                              { return sum + s.size() + 1; }));
    int counter = 0;
    for (auto& name: cnames)
    {
        std::strcpy(names.get() + counter, name.c_str());
        counter += static_cast<int>(name.size()) + 1;
    }
    return XPRSaddnames(prob, type, names.get(), first, end);
}
} // namespace

void SolverXpress::add_names(int type, const std::vector<std::string>& cnames, int first, int end)
{
    ::add_names(_xprs, type, cnames, first, end);
}

void SolverXpress::chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj)
{
    assert(obj.size() == mindex.size());
    int status = XPRSchgobj(_xprs, static_cast<int>(obj.size()), mindex.data(), obj.data());
    zero_status_check(status, "change objective", LOGLOCATION);
}

void SolverXpress::chg_obj_direction(const bool minimize)
{
    int objsense = minimize ? XPRS_OBJ_MINIMIZE : XPRS_OBJ_MAXIMIZE;
    int status = XPRSchgobjsense(_xprs, objsense);
    zero_status_check(status, "change objective sense", LOGLOCATION);
}

void SolverXpress::chg_bounds(const std::vector<int>& mindex,
                              const std::vector<char>& qbtype,
                              const std::vector<double>& bnd)
{
    assert(qbtype.size() == mindex.size());
    assert(bnd.size() == mindex.size());
    int status = XPRSchgbounds(_xprs,
                               static_cast<int>(mindex.size()),
                               mindex.data(),
                               qbtype.data(),
                               bnd.data());
    zero_status_check(status, "change bounds", LOGLOCATION);
}

void SolverXpress::chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype)
{
    assert(qctype.size() == mindex.size());
    int status = XPRSchgcoltype(_xprs,
                                static_cast<int>(mindex.size()),
                                mindex.data(),
                                qctype.data());
    zero_status_check(status, "change column types", LOGLOCATION);
}

void SolverXpress::chg_rhs_values(std::vector<int>& id_rows, std::vector<double>& vals)
{

    int status = XPRSchgrhs(_xprs, id_rows.size(), id_rows.data(), vals.data());

    zero_status_check(status, "change rhs", LOGLOCATION);
}

void SolverXpress::chg_coefs(const std::vector<int>& id_rows,
                             const std::vector<int>& id_cols,
                             const std::vector<double>& vals)
{
    int status = XPRSchgmcoef(_xprs,
                              static_cast<int>(id_rows.size()),
                              const_cast<int*>(id_rows.data()),
                              const_cast<int*>(id_cols.data()),
                              const_cast<double*>(vals.data()));
    zero_status_check(status, "change matrix coefficient", LOGLOCATION);
}

void SolverXpress::chg_rhs(int id_row, double val)
{
    int status = XPRSchgrhs(_xprs,
                            1,
                            std::vector<int>(1, id_row).data(),
                            std::vector<double>(1, val).data());
    zero_status_check(status, "change rhs", LOGLOCATION);
}

void SolverXpress::chg_coef(int id_row, int id_col, double val)
{
    int status = XPRSchgcoef(_xprs, id_row, id_col, val);
    zero_status_check(status, "change matrix coefficient", LOGLOCATION);
}

void SolverXpress::chg_row_name(int id_row, const std::string& name)
{
    int status = XPRSaddnames(_xprs, 1, name.data(), id_row, id_row);
    zero_status_check(status, "Set row name", LOGLOCATION);
}

void SolverXpress::chg_col_name(int id_col, const std::string& name)
{
    int status = XPRSaddnames(_xprs, 2, name.data(), id_col, id_col);
    zero_status_check(status, "Set col name", LOGLOCATION);
}

void SolverXpress::mark_indices_to_keep_presolve(int nrows, int ncols, int* rowind, int* colind)
{
    // From FICO Xpress API page
    // Allows the user to mark rows and columns in order to prevent the presolve removing these rows
    // and columns from the matrix
    int status = XPRSloadsecurevecs(_xprs, nrows, ncols, rowind, colind);
    zero_status_check(status, "keep presolve from removing rows and columns", LOGLOCATION);
}

/*************************************************************************************************
-----------------------------    Methods to solve the problem
---------------------------------
*************************************************************************************************/
void SolverXpress::presolve_only()
{
    set_simplex_iter(0);

    int status = XPRSlpoptimize(_xprs, "");
    zero_status_check(status, "presolve problem as LP", LOGLOCATION);

    // TODO Return a status as well?
}

int SolverXpress::solve_lp()
{
    int lp_status;
    int status = XPRSlpoptimize(_xprs, "");
    zero_status_check(status, "solve problem as LP", LOGLOCATION);

    int xprs_status(0);
    status = XPRSgetintattrib(_xprs, XPRS_LPSTATUS, &xprs_status);
    zero_status_check(status, "get LP status after LP solve", LOGLOCATION);

    if (xprs_status == XPRS_LP_OPTIMAL)
    {
        lp_status = OPTIMAL;
    }
    else if (xprs_status == XPRS_LP_INFEAS)
    {
        lp_status = INFEASIBLE;
    }
    else if (xprs_status == XPRS_LP_UNBOUNDED)
    {
        lp_status = UNBOUNDED;
    }
    else
    {
        lp_status = UNKNOWN;
        std::ostringstream err;
        err << LOGLOCATION << "Error : UNKNOWN XPRESS STATUS IS : " << xprs_status << std::endl;
        std::cerr << err.str();
    }
    return lp_status;
}

int SolverXpress::solve_mip()
{
    int lp_status;
    int status(0);
    status = XPRSmipoptimize(_xprs, "");
    zero_status_check(status, "solve problem as MIP", LOGLOCATION);

    int xprs_status(0);
    status = XPRSgetintattrib(_xprs, XPRS_MIPSTATUS, &xprs_status);
    zero_status_check(status, "get MIP status after MIP solve", LOGLOCATION);

    if (xprs_status == XPRS_MIP_OPTIMAL)
    {
        lp_status = OPTIMAL;
    }
    else if (xprs_status == XPRS_MIP_INFEAS)
    {
        lp_status = INFEASIBLE;
    }
    else if (xprs_status == XPRS_MIP_UNBOUNDED)
    {
        lp_status = UNBOUNDED;
    }
    else
    {
        lp_status = UNKNOWN;
        std::ostringstream err;
        err << LOGLOCATION << "XPRESS STATUS IS : " << xprs_status << std::endl;
        std::cerr << err.str();
    }
    return lp_status;
}

/*************************************************************************************************
-------------------------    Methods to get solutions information
-----------------------------
*************************************************************************************************/
void SolverXpress::get_basis(int* rstatus, int* cstatus) const
{
    int status = XPRSgetbasis(_xprs, rstatus, cstatus);
    zero_status_check(status, "get basis", LOGLOCATION);
}

double SolverXpress::get_mip_value() const
{
    double val;
    int status = XPRSgetdblattrib(_xprs, XPRS_MIPOBJVAL, &val);
    zero_status_check(status, "get MIP value", LOGLOCATION);
    return val;
}

double SolverXpress::get_lp_value() const
{
    double val;
    int status = XPRSgetdblattrib(_xprs, XPRS_LPOBJVAL, &val);
    zero_status_check(status, "get LP value", LOGLOCATION);
    return val;
}

int SolverXpress::get_splex_num_of_ite_last() const
{
    int result;
    int status = XPRSgetintattrib(_xprs, XPRS_SIMPLEXITER, &result);
    zero_status_check(status, "get simplex iterations", LOGLOCATION);
    return result;
}

void SolverXpress::get_lp_sol(double* primals, double* duals, double* reduced_costs) const
{
    int status = XPRSgetlpsol(_xprs, primals, nullptr, duals, reduced_costs);
    zero_status_check(status, "get LP sol", LOGLOCATION);
}

void SolverXpress::get_mip_sol(double* primals)
{
    int status = XPRSgetmipsol(_xprs, primals, nullptr);
    zero_status_check(status, "get MIP sol", LOGLOCATION);
}

void SolverXpress::get_presolve_map(int* rowmap, int* colmap) const
{
    int status = XPRSgetpresolvemap(_xprs, rowmap, colmap);
    zero_status_check(status, "get presolve map", LOGLOCATION);
}

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels
---------------------------
*************************************************************************************************/
void SolverXpress::set_output_log_level(int loglevel)
{
    int status = XPRSsetcbmessage(_xprs, optimizermsg, &get_stream());
    zero_status_check(status, "set message stream to solver stream", LOGLOCATION);

    if (loglevel > 1)
    {
        int status = XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_FULL_OUTPUT);
        zero_status_check(status, "set log level", LOGLOCATION);
    }
    else
    {
        int status = XPRSsetintcontrol(_xprs, XPRS_OUTPUTLOG, XPRS_OUTPUTLOG_NO_OUTPUT);
        zero_status_check(status, "set log level", LOGLOCATION);
    }
}

void SolverXpress::set_algorithm(const std::string& algo)
{
    if (algo == "BARRIER")
    {
        int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
        zero_status_check(status, "set barrier algorithm", LOGLOCATION);
    }
    else if (algo == "BARRIER_WO_CROSSOVER")
    {
        int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 4);
        zero_status_check(status, "set barrier algorithm", LOGLOCATION);
        status = XPRSsetintcontrol(_xprs, XPRS_CROSSOVER, 0);
        zero_status_check(status, "desactivate barrier crossover", LOGLOCATION);
    }
    else if (algo == "DUAL")
    {
        int status = XPRSsetintcontrol(_xprs, XPRS_DEFAULTALG, 2);
        zero_status_check(status, "set dual simplex algorithm", LOGLOCATION);
    }
    else
    {
        throw InvalidSolverOptionException("set_algorithm : " + algo, LOGLOCATION);
    }
}

void SolverXpress::set_threads(int n_threads)
{
    int status = XPRSsetintcontrol(_xprs, XPRS_THREADS, n_threads);
    zero_status_check(status, "set threads", LOGLOCATION);
}

void SolverXpress::set_optimality_gap(double gap)
{
    int status = XPRSsetdblcontrol(_xprs, XPRS_OPTIMALITYTOL, gap);
    zero_status_check(status, "set optimality gap", LOGLOCATION);
}

void SolverXpress::set_simplex_iter(int iter)
{
    int status = XPRSsetintcontrol(_xprs, XPRS_BARITERLIMIT, iter);
    zero_status_check(status, "set barrier max iter", LOGLOCATION);

    status = XPRSsetintcontrol(_xprs, XPRS_LPITERLIMIT, iter);
    zero_status_check(status, "set simplex max iter", LOGLOCATION);
}

void SolverXpress::save_prob(const std::filesystem::path& filename)
{
    std::filesystem::path filename_to_use{filename};
    filename_to_use.replace_extension(".svf");
    int status = XPRSsaveas(_xprs, filename_to_use.string().c_str());
    if (status != 0)
    {
        char errmsg[512];
        XPRSgetlasterror(_xprs, errmsg);
        std::cerr << errmsg << std::endl;
    }
    zero_status_check(status, "save problem", LOGLOCATION);
}

void SolverXpress::restore_prob(const std::filesystem::path& filename)
{
    int status = XPRSrestore(_xprs, filename.string().c_str(), "");
    std::string message = "restore problem at : " + filename.string();
    zero_status_check(status, message, LOGLOCATION);
}

void XPRS_CC optimizermsg(XPRSprob prob, void* strPtr, const char* sMsg, int nLen, int nMsglvl)
{
    std::list<std::ostream*>* ptr = nullptr;
    if (strPtr != nullptr)
    {
        ptr = (std::list<std::ostream*>*)strPtr;
    }
    switch (nMsglvl)
    {
        /* Print Optimizer error messages and warnings */
    case 4: /* error */
    case 3: /* warning */
    case 2: /* dialogue */
    case 1: /* information */
        if (ptr != nullptr)
        {
            for (const auto& stream: *ptr)
            {
                *stream << sMsg << std::endl;
            }
        }
        break;
        /* Exit and flush buffers */
    default:
        fflush(nullptr);
        break;
    }
}

void errormsg(XPRSprob& _xprs, const char* sSubName, int nLineNo, int nErrCode)
{
    int nErrNo; /* Optimizer error number */
                /* Print error message */
    printf("The subroutine %s has failed on line %d\n", sSubName, nLineNo);

    /* Append the error code if it exists */
    if (nErrCode != -1)
    {
        printf("with error code %d.\n\n", nErrCode);
    }

    /* Append Optimizer error number, if available */
    if (nErrCode == 32)
    {
        XPRSgetintattrib(_xprs, XPRS_ERRORCODE, &nErrNo);
        printf("The Optimizer eror number is: %d\n\n", nErrNo);
    }

    /* Free memory close files and exit */
    XPRSdestroyprob(_xprs);
    XPRSfree();
    exit(nErrCode);
}

namespace
{
void debug_print_duplicates(const std::vector<std::string>& names, const char* element_type)
{
    std::map<std::string, std::vector<size_t>> name_to_indexes;
    for (size_t i = 0; i < names.size(); ++i)
    {
        name_to_indexes[names[i]].push_back(i);
    }
    for (const auto& [name, indexes]: name_to_indexes)
    {
        if (indexes.size() > 1)
        {
            std::cerr << "Duplicate " << element_type << " name: '" << name << "' at indexes: ";
            for (size_t idx = 0; idx < indexes.size(); ++idx)
            {
                std::cerr << indexes[idx];
                if (idx + 1 < indexes.size())
                {
                    std::cerr << ", ";
                }
            }
            std::cerr << std::endl;
        }
    }
    // Check for duplicate names
    std::set<std::string> name_set;
    for (const auto& name: names)
    {
        if (!name_set.insert(name).second)
        {
            throw std::runtime_error(std::string("Duplicate ") + element_type
                                     + " name found: " + name);
        }
    }
}
} // namespace

XPRSprob SolverXpress::clone_matrix_to_new_prob() const
{
    std::lock_guard guard(mutex_); // Ensure thread safety during cloning
    int ncols = get_ncols();
    int nrows = get_nrows();
    int nelems = get_nelems();

    // Allocate arrays for matrixÉZA"QAZ data
    std::vector<char> rowtype(nrows);
    std::vector<double> rhs(nrows);
    std::vector<double> objcoef(ncols);
    std::vector<int> mstart(ncols + 1);
    std::vector<int> mrwind(nelems);
    std::vector<double> dmatval(nelems);
    std::vector<double> lb(ncols);
    std::vector<double> ub(ncols);
    std::vector<char> coltype(ncols);

    // Fill arrays from the source solver
    get_row_type(rowtype.data(), 0, nrows - 1);
    get_rhs(rhs.data(), 0, nrows - 1);
    get_obj(objcoef.data(), 0, ncols - 1);
    int nels = 0;
    get_cols(mstart.data(), mrwind.data(), dmatval.data(), nelems, &nels, 0, ncols - 1);
    get_lb(lb.data(), 0, ncols - 1);
    get_ub(ub.data(), 0, ncols - 1);
    get_col_type(coltype.data(), 0, ncols - 1);

    // Get names
    std::vector<std::string> row_names = get_row_names(0, nrows - 1);
    std::vector<std::string> col_names = get_col_names(0, ncols - 1);

    // Create new problem
    XPRSprob new_prob = nullptr;
    int status = XPRScreateprob(&new_prob);
    zero_status_check(status, "create XPRESS problem (clone_matrix_to_new_prob)", LOGLOCATION);

    // Load the matrix into the new problem
    status = XPRSloadlp(new_prob,
                        "cloned_prob", // name
                        ncols,
                        nrows,
                        rowtype.data(),
                        rhs.data(),
                        nullptr, // rng (not used)
                        objcoef.data(),
                        mstart.data(),
                        nullptr, // collen (not used)
                        mrwind.data(),
                        dmatval.data(),
                        lb.data(),
                        ub.data());
    zero_status_check(status, "XPRSloadlp (clone_matrix_to_new_prob)", LOGLOCATION);

    // Set column types
    std::vector<int> col_indices(ncols);
    std::iota(col_indices.begin(), col_indices.end(), 0);
    status = XPRSchgcoltype(new_prob, ncols, col_indices.data(), coltype.data());
    zero_status_check(status,
                      "set column types in new XPRESS problem (clone_matrix_to_new_prob)",
                      LOGLOCATION);

    // Helper lambda to handle names (row or col)
    auto handle_names = [](XPRSprob prob,
                           int type,
                           const std::vector<std::string>& names,
                           int n_elements,
                           const char* element_type)
    {
        if (!names.empty())
        {
            if (names.size() != static_cast<size_t>(n_elements))
            {
                throw std::runtime_error(std::string("Nombre de noms de ") + element_type
                                         + " incohérent");
            }
#ifdef NDEBUG
            // Release mode: skip duplicate checks
#else
            // Print all duplicates and their indexes
            debug_print_duplicates(names, element_type);
#endif
            int status = ::add_names(prob, type, names, 0, n_elements - 1);
            zero_status_check(status,
                              std::string("set ") + element_type
                                + " names in new XPRESS problem (clone_matrix_to_new_prob)",
                              LOGLOCATION);
        }
    };

    // Use the lambda for rows and columns
    handle_names(new_prob, ROW, row_names, nrows, "row");
    handle_names(new_prob, COLUMN, col_names, ncols, "column");

    return new_prob;
}
