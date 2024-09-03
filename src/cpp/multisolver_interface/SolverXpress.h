#pragma once

#include <fstream>
#include <mutex>

#include "multisolver_interface/SolverAbstract.h"
#include "multisolver_interface/environment.h"

/*!
 * \class class SolverXpress
 * \brief Daughter class of AsbtractSolver implementing solver XPRESS FICO
 */
class SolverXpress : public SolverAbstract {
  /*************************************************************************************************
  ----------------------------------------    ATTRIBUTES
  ---------------------------------------
  *************************************************************************************************/
  static int
      _NumberOfProblems; /*!< Counter of the total number of Cplex problems
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
  SolverXpress(SolverLogManager &log_manager);

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
  SolverXpress(const SolverXpress &other) = delete;
  SolverXpress &operator=(const SolverXpress &other) = delete;

  virtual ~SolverXpress();
  virtual int get_number_of_instances() override;

  virtual std::string get_solver_name() const override { return name_; }

  /*************************************************************************************************
  ---------------------------------    Output and stream management
  -----------------------------
  *************************************************************************************************/

  /*************************************************************************************************
  ------    Destruction or creation of inner strctures and datas, closing
  environments    ----------
  *************************************************************************************************/
 public:
  virtual void init() override;
  virtual void free() override;

  /*************************************************************************************************
  -------------------------------    Reading & Writing problems
  -------------------------------
  *************************************************************************************************/
 public:
  virtual void write_prob_mps(const std::filesystem::path &filename) override;
  virtual void write_prob_lp(const std::filesystem::path &filename) override;
  virtual void write_basis(const std::filesystem::path &filename) override;

  virtual void read_prob_mps(const std::filesystem::path &filename,
                             bool compressed) override;
  virtual void read_prob_lp(const std::filesystem::path &filename) override;
  virtual void read_basis(const std::filesystem::path &filename) override;

  virtual void copy_prob(const SolverAbstract::Ptr fictif_solv) override;

 private:
  virtual void read_prob(const char *prob_name, const char *flags);

  /*************************************************************************************************
  -----------------------    Get general informations about problem
  ----------------------------
  *************************************************************************************************/
 public:
  virtual int get_ncols() const override;
  virtual int get_nrows() const override;
  virtual int get_nelems() const override;
  virtual int get_n_integer_vars() const override;
  virtual void get_obj(double *obj, int first, int last) const override;
  void set_obj_to_zero() override;
  void set_obj(const double *obj, int first, int last) override;
  virtual void get_rows(int *mstart, int *mclind, double *dmatval, int size,
                        int *nels, int first, int last) const override;
  virtual void get_row_type(char *qrtype, int first, int last) const override;
  virtual void get_rhs(double *rhs, int first, int last) const override;
  virtual void get_rhs_range(double *range, int first, int last) const override;
  virtual void get_col_type(char *coltype, int first, int last) const override;
  virtual void get_lb(double *lb, int fisrt, int last) const override;
  virtual void get_ub(double *ub, int fisrt, int last) const override;

  virtual int get_row_index(std::string const &name) override;
  virtual int get_col_index(std::string const &name) override;
  virtual std::vector<std::string> get_row_names(int first, int last) override;
  virtual std::vector<std::string> get_row_names() override;
  virtual std::vector<std::string> get_col_names(int first, int last) override;
  virtual std::vector<std::string> get_col_names() override;

  /*************************************************************************************************
  ------------------------------    Methods to modify problem
  ----------------------------------
  *************************************************************************************************/
 public:
  virtual void del_rows(int first, int last) override;
  virtual void add_rows(int newrows, int newnz, const char *qrtype,
                        const double *rhs, const double *range,
                        const int *mstart, const int *mclind,
                        const double *dmatval,
                        const std::vector<std::string> &names = {}) override;
  virtual void add_cols(int newcol, int newnz, const double *objx,
                        const int *mstart, const int *mrwind,
                        const double *dmatval, const double *bdl,
                        const double *bdu) override;
  virtual void add_name(int type, const char *cnames, int indice) override;
  virtual void add_names(int type, const std::vector<std::string> &cnames,
                         int first, int end) override;
  virtual void chg_obj(const std::vector<int> &mindex,
                       const std::vector<double> &obj) override;
  virtual void chg_obj_direction(const bool minimize) override;
  virtual void chg_bounds(const std::vector<int> &mindex,
                          const std::vector<char> &qbtype,
                          const std::vector<double> &bnd) override;
  virtual void chg_col_type(const std::vector<int> &mindex,
                            const std::vector<char> &qctype) override;
  virtual void chg_rhs(int id_row, double val) override;
  virtual void chg_coef(int id_row, int id_col, double val) override;
  virtual void chg_row_name(int id_row, std::string const &name) override;
  virtual void chg_col_name(int id_col, std::string const &name) override;

  /*************************************************************************************************
  -----------------------------    Methods to solve the problem
  ---------------------------------
  *************************************************************************************************/
 public:
  virtual int solve_lp() override;
  virtual int solve_mip() override;

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
  virtual void get_basis(int *rstatus, int *cstatus) const override;
  void SetBasis(std::vector<int> rstatus, std::vector<int> cstatus) override;
  virtual double get_mip_value() const override;
  virtual double get_lp_value() const override;
  virtual int get_splex_num_of_ite_last() const override;
  virtual void get_lp_sol(double *primals, double *duals,
                          double *reduced_costs) override;
  virtual void get_mip_sol(double *primals) override;

  /*************************************************************************************************
  ------------------------    Methods to set algorithm or logs levels
  ---------------------------
  *************************************************************************************************/
 public:
  void set_output_log_level(int loglevel) final;
  virtual void set_algorithm(std::string const &algo) override;
  virtual void set_threads(int n_threads) override;
  virtual void set_optimality_gap(double gap) override;
  virtual void set_simplex_iter(int iter) override;
  void chg_col_names(int id_col, const std::vector<std::string> &name) override;
  void chg_row_names(int id_col,
                     const std::vector<std::string> &names) override;

 public:
  std::ofstream _log_stream;

 private:
  std::vector<std::string> get_names(int type, size_t nelements);
};

/************************************************************************************\
* Name:         optimizermsg *
* Purpose:      Display Optimizer error messages and warnings. *
* Arguments:    const char *sMsg    Message string *
*               int nLen            Message length *
*               int nMsgLvl         Message type *
* Return Value: None. *
\************************************************************************************/
void errormsg(XPRSprob &xprs, const char *sSubName, int nLineNo, int nErrCode);

/************************************************************************************\
* Name:         errormsg *
* Purpose:      Display error information about failed subroutines. *
* Arguments:    const char *sSubName    Subroutine name *
*               int nLineNo             Line number *
*               int nErrCode            Error code *
* Return Value: None *
\************************************************************************************/
void optimizermsg(XPRSprob prob, void *stream, const char *sMsg, int nLen,
                  int nMsglvl);