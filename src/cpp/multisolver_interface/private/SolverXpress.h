#pragma once

#include <fstream>
#include <mutex>

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"
#include "antares-xpansion/multisolver_interface/environment.h"

/*!
 * \class class SolverXpress
 * \brief Daughter class of AsbtractSolver implementing solver XPRESS FICO
 */
class SolverXpress: public SolverAbstract
{
    /*************************************************************************************************
    ----------------------------------------    ATTRIBUTES
    ---------------------------------------
    *************************************************************************************************/
    static int _NumberOfProblems; /*!< Counter of the total number of Cplex problems
                                  declared to set or end the environment */
    static std::mutex license_guard;

public:
    XPRSprob _xprs; /*!< Problem in XPRESS */
    const std::string name_ = "XPRESS";

    /*************************************************************************************************
    -----------------------------------    Constructor/Desctructor
    --------------------------------
    *************************************************************************************************/

public:
    /**
     * @brief Default constructor of a XPRESS solver
     */
    SolverXpress();
    SolverXpress(const SolverLogManager& log_manager);

    /**
     * @brief Copy constructor of XPRESS, copy the problem toCopy in memory and
     * name it "name"
     *
     * @param toCopy : Pointer to an AbstractSolver object, containing an XPRESS
     * solver to copy
     */
    explicit SolverXpress(const SolverAbstract::Ptr toCopy);
    explicit SolverXpress(const std::shared_ptr<const SolverAbstract> toCopy);

    /*SolverXpress ctor accept only std::shared_ptr*/
    SolverXpress(const SolverXpress& other) = delete;
    SolverXpress& operator=(const SolverXpress& other) = delete;

    ~SolverXpress();
    int get_number_of_instances() override;

    std::string get_solver_name() const override
    {
        return name_;
    }

    /*************************************************************************************************
    ---------------------------------    Output and stream management
    -----------------------------
    *************************************************************************************************/

    /*************************************************************************************************
    ------    Destruction or creation of inner strctures and datas, closing
    environments    ----------
    *************************************************************************************************/

public:
    void init() override;
    void free() override;

    /*************************************************************************************************
    -------------------------------    Reading & Writing problems
    -------------------------------
    *************************************************************************************************/

public:
    void write_prob_mps(const std::filesystem::path& filename) override;
    void write_prob_lp(const std::filesystem::path& filename) override;
    void save_prob(const std::filesystem::path& filename) override;
    void write_basis(const std::filesystem::path& filename) override;

    void read_prob_mps(const std::filesystem::path& filename) override;
    void read_prob_lp(const std::filesystem::path& filename) override;
    void restore_prob(const std::filesystem::path& filename) override;
    void read_basis(const std::filesystem::path& filename) override;
    void set_basis(std::span<int> rstatus, std::span<int> cstatus) override;

    void copy_prob(const SolverAbstract::Ptr fictif_solv) override;

private:
    void read_prob(const char* prob_name, const char* flags);

    /*************************************************************************************************
    -----------------------    Get general informations about problem
    ----------------------------
    *************************************************************************************************/

public:
    int get_ncols() const override;
    int get_nrows() const override;
    int get_nelems() const override;
    int get_n_integer_vars() const override;
    void get_obj(double* obj, int first, int last) const override;
    void set_obj_to_zero() override;
    void set_obj(const double* obj, int first, int last) override;
    void get_rows(int* mstart,
                  int* mclind,
                  double* dmatval,
                  int size,
                  int* nels,
                  int first,
                  int last) const override;
    void get_row_type(char* qrtype, int first, int last) const override;
    void get_rhs(double* rhs, int first, int last) const override;
    void get_rhs_range(double* range, int first, int last) const override;
    void get_col_type(char* coltype, int first, int last) const override;
    void get_lb(double* lb, int fisrt, int last) const override;
    void get_ub(double* ub, int fisrt, int last) const override;

    int get_row_index(const std::string& name) override;
    int get_col_index(const std::string& name) override;
    std::vector<std::string> get_row_names(int first, int last) override;
    std::vector<std::string> get_row_names() override;
    std::vector<std::string> get_col_names(int first, int last) override;
    std::vector<std::string> get_col_names() override;

    /*************************************************************************************************
    ------------------------------    Methods to modify problem
    ----------------------------------
    *************************************************************************************************/

public:
    void del_rows(int first, int last) override;
    void add_rows(int newrows,
                  int newnz,
                  const char* qrtype,
                  const double* rhs,
                  const double* range,
                  const int* mstart,
                  const int* mclind,
                  const double* dmatval,
                  const std::vector<std::string>& row_names) override;
    void add_cols(int newcol,
                  int newnz,
                  const double* objx,
                  const int* mstart,
                  const int* mrwind,
                  const double* dmatval,
                  const double* bdl,
                  const double* bdu,
                  const std::vector<std::string>& col_names) override;
    void add_name(int type, const char* cnames, int indice) override;
    void add_names(int type, const std::vector<std::string>& cnames, int first, int end) override;
    void chg_obj(const std::vector<int>& mindex, const std::vector<double>& obj) override;
    void chg_obj_direction(const bool minimize) override;
    void chg_bounds(const std::vector<int>& mindex,
                    const std::vector<char>& qbtype,
                    const std::vector<double>& bnd) override;
    void chg_col_type(const std::vector<int>& mindex, const std::vector<char>& qctype) override;
    void chg_rhs(int id_row, double val) override;
    void chg_coef(int id_row, int id_col, double val) override;
    void chg_row_name(int id_row, const std::string& name) override;
    void chg_col_name(int id_col, const std::string& name) override;

    /*************************************************************************************************
    -----------------------------    Methods to solve the problem
    ---------------------------------
    *************************************************************************************************/

public:
    int solve_lp() override;
    int solve_mip() override;

    /*************************************************************************************************
    -------------------------    Methods to get solutions information
    -----------------------------
    *************************************************************************************************/

public:
    /**
    * @brief Returns the current basis into the user’s data arrays.
    *
    * @param rstatus    : Integer array of length ROWS to the basis status of the
    slack, surplus or artifficial variable associated with each row. The status
    will be one of: 0 slack, surplus or artifficial is non-basic at lower bound;
                        1 slack, surplus or artifficial is basic;
                        2 slack or surplus is non-basic at upper bound.
                        3 slack or surplus is super-basic.
                        May be NULL if not required.
    * @param cstatus    : Integer array of length COLS to hold the basis status of
    the columns in the constraint matrix. The status will be one of: 0 variable is
    non-basic at lower bound, or superbasic at zero if the variable has no lower
    bound; 1 variable is basic; 2 variable is non-basic at upper bound; 3 variable
    is super-basic. May be NULL if not required.
    */
    void get_basis(int* rstatus, int* cstatus) const override;
    double get_mip_value() const override;
    double get_lp_value() const override;
    int get_splex_num_of_ite_last() const override;
    void get_lp_sol(double* primals, double* duals, double* reduced_costs) const override;
    void get_mip_sol(double* primals) override;

    /*************************************************************************************************
    ------------------------    Methods to set algorithm or logs levels
    ---------------------------
    *************************************************************************************************/

public:
    void set_output_log_level(int loglevel) final;
    void set_algorithm(const std::string& algo) override;
    void set_threads(int n_threads) override;
    void set_optimality_gap(double gap) override;
    void set_simplex_iter(int iter) override;

public:
    std::ofstream _log_stream;

private:
    std::vector<std::string> get_names(int type, int n_elements);
};

/************************************************************************************\
* Name:         optimizermsg *
* Purpose:      Display Optimizer error messages and warnings. *
* Arguments:    const char *sMsg    Message string *
*               int nLen            Message length *
*               int nMsgLvl         Message type *
* Return Value: None. *
\************************************************************************************/
void errormsg(XPRSprob& xprs, const char* sSubName, int nLineNo, int nErrCode);

/************************************************************************************\
* Name:         errormsg *
* Purpose:      Display error information about failed subroutines. *
* Arguments:    const char *sSubName    Subroutine name *
*               int nLineNo             Line number *
*               int nErrCode            Error code *
* Return Value: None *
\************************************************************************************/
void optimizermsg(XPRSprob prob, void* stream, const char* sMsg, int nLen, int nMsglvl);
