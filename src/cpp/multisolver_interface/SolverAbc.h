#include "Abc.h"
#include "multisolver_interface/SolverAbstract.h"
class SolverAbc : public SolverAbstract {
  /**
   * @brief Returns number of instances of solver currently in memory
   */
  Abc abc_;
  int get_number_of_instances() override {
    return abc_.ReturnInt();
    ;
    ;
    ;
  }
  /**
   * @brief Returns the solver used
   */
  std::string get_solver_name() const override { return "Abc"; }
  /**
   * @brief Initializes a problem
   */
  void init() override { abc_.ReturnVoid(); };
  /**
   * @brief Frees all the datas contained in the Solver environment
   */
  void free() override { abc_.ReturnVoid(); }
  /**
   * @brief writes an optimization problem in a MPS file
   *
   * @param name   : name of the file to write
   */
  void write_prob_mps(const std::filesystem::path &filename) override {
    abc_.ReturnVoid();
  };
  /**
   * @brief writes an optimization problem in a LP file
   *
   * @param name   : name of the file to write
   */
  void write_prob_lp(const std::filesystem::path &filename) override {
    abc_.ReturnVoid();
  };
  /**
   * @brief Writes the current basis to a file for later input into the
   * optimizer
   *
   * @param filename    : file name where the basis is written
   */
  void write_basis(const std::filesystem::path &filename) override {}

  /**
   * @brief reads an optimization problem contained in a MPS file
   *
   * @param name   : name of the file to read
   */
  void read_prob_mps(const std::filesystem::path &filename) override {}

  /**
   * @brief reads an optimization problem contained in a MPS file
   *
   * @param name   : name of the file to read
   */
  void read_prob_lp(const std::filesystem::path &filename) override {}

  /**
   * @brief Instructs the optimizer to read in a previously saved basis from a
   * file
   *
   * @param filename: File name where the basis is to be read
   */
  void read_basis(const std::filesystem::path &filename) override {}

  /**
   * @brief copy an existing problem
   *
   * @param prb_to_copy : Ptr to the
   */
  void copy_prob(Ptr fictif_solv) override {}

  /*************************************************************************************************
  -----------------------    Get general informations about problem
  ----------------------------
  *************************************************************************************************/

  /**
   * @brief returns number of columns of the problem
   */
  int get_ncols() const override { return abc_.ReturnInt(); }

  /**
   * @brief returns number of rows of the problem
   */
  int get_nrows() const override { abc_.ReturnInt(); }

  /**
   * @brief returns number of non zeros elements in the matrix, excluding
   * objective
   */
  int get_nelems() const override { abc_.ReturnInt(); }

  /**
   * @brief returns number of integer variables in the problem
   */
  int get_n_integer_vars() const override { abc_.ReturnInt(); }

  /**
   * @brief returns the objective function coefficients for the columns in a
   * given range
   */
  void get_obj(double *obj, int first, int last) const override {}

  /**
  * @brief get coefficients of rows from index first to last
  *
  * @param mstart     : Integer array which will be filled with the indices
  indicating the starting offsets in the mclind and dmatval arrays for each
  requested row.
  * @param mclind     : array containing the column indices of the elements
  contained in dmatval
  * @param dmatval    : array containing non zero elements of the rows
  * @param size       : maximum number of elements to be retrieved
  * @param nels       : array containing number of elements in dmatval and
  mclind
  * @param first      : first index of row to get
  * @param last       : last index of row to get
  */
  void get_rows(int *mstart, int *mclind, double *dmatval, int size, int *nels,
                int first, int last) const override {}

  /**
  * @brief Returns the row types for the rows in a given range.
  *
  * @param qrtype : Character array of length last-first+1 characters where the
  row types will be returned (N, E, L, G, R)
  * @param first  : first index of row to get
  * @param last   : last index of row to get
  */
  void get_row_type(char *qrtype, int first, int last) const override {}

  /**
   * @brief Returns the right-hand sides of the rows in a given range.
   *
   * @param qrtype : Character array of length last-first+1 characters where the
   * rhs will be stored
   * @param first  : first index of row to get
   * @param last   : last index of row to get
   */
  void get_rhs(double *rhs, int first, int last) const override {}

  /**
  * @brief Returns the right hand side range values for the rows in a given
  range.
  *
  * @param range  : Double array of length last-first+1 where the right hand
  side range values are to be placed.
  * @param first  : first index of row to get
  * @param last   : last index of row to get
  */
  void get_rhs_range(double *range, int first, int last) const override {}

  /**
  * @brief Returns the column types for the columns in a given range.
  *
  * @param coltype    : Character array of length last-first+1 where the column
  types will be returned (C: continuous, I: integer, B: binary, S:
  semi-continuous, R: semi-continuous integer, P: partial integer)
  * @param first      : first index of row to get
  * @param last       : last index of row to get
  */
  void get_col_type(char *coltype, int first, int last) const override {}

  /**
   * @brief Returns the lower bounds for variables in a given range.
   *
   * @param lb     : Double array of length last-first+1 where the lower bounds
   * are to be placed.
   * @param first  : First column in the range.
   * @param last   : Last column in the range.
   */
  void get_lb(double *lb, int fisrt, int last) const override {}

  /**
   * @brief Returns the upper bounds for variables in a given range.
   *
   * @param lb     : Double array of length last-first+1 where the upper bounds
   * are to be placed.
   * @param first  : First column in the range.
   * @param last   : Last column in the range.
   */
  void get_ub(double *ub, int fisrt, int last) const override {}

  /**
   * @brief Returns the index of row named "name"
   *
   * @param name : name of row to get the index
   */
  int get_row_index(std::string const &name) const override {}

  /**
   * @brief Returns the index of column named "name"
   *
   * @param name : name of column to get the index
   */
  int get_col_index(std::string const &name) const override {}

  /**
   * @brief Returns the names of row from index first to last
   * cannot be declared as const because of some solver methods
   *
   * @param first : first index from which name has be returned
   * @param last  : last index from which name has be returned
   * @return names : vector of names
   */
  std::vector<std::string> get_row_names(int first, int last) override {
    return {""};
  }

  /**
   * @brief Returns the names of columns from index first to last
   * cannot be declared as const because of some solver methods
   *
   * @param first : first index from which name has be returned
   * @param last  : last index from which name has be returned
   * @return names : vector of names
   */
  std::vector<std::string> get_col_names(int first, int last) override {
    return {""};
  }

  /*************************************************************************************************
  ------------------------------    Methods to modify problem
  ----------------------------------
  *************************************************************************************************/

  /**
   * @brief Deletes rows between index first and last
   *
   * @param first  : first row index to delete
   * @param last   : last row index to delete
   */
  void del_rows(int first, int last) override {}

  /**
  * @brief Adds rows to the problem
  *
  * @param newrows    : Number of new rows.
  * @param newnz      : Number of new nonzeros in the added rows.
  * @param qrtype     : Character array of length newrow containing the row
  types:
  * @param rhs        : Double array of length newrow containing the right hand
  side elements.
  * @param range      : Double array of length newrow containing the row range
  elements (index read only for R type rows)
  * @param mstart     : Integer array of length newrow + 1 containing the
  offsets in the mclind and dmatval arrays of the start of the elements for each
  row.
  * @param mclind     : Integer array of length newnz containing the
  (contiguous) column indices for the elements in each row.
  * @param dmatval    : Double array of length newnz containing the (contiguous)
  element values.
  */
  void add_rows(int newrows, int newnz, const char *qrtype, const double *rhs,
                const double *range, const int *mstart, const int *mclind,
                const double *dmatval) override {}

  /**
  * @brief Adds new columns to the problem
  *
  * @param newcol     : Number of new columns.
  * @param newnz      : Number of new nonzeros in the added columns.
  * @param objx       : Double array of length newcol containing the objective
  function coefficients of the new columns.
  * @param mstart     : Integer array of length newcol containing the offsets in
  the mrwind and dmatval arrays of the start of the elements for each column.
  * @param mrwind     : Integer array of length newnz containing the row indices
  for the elements 7 in each column.
  * @param dmatval    : Double array of length newnz containing the element
  values.
  * @param bdl        : Double array of length newcol containing the lower
  bounds on the added columns.
  * @param bdu        : Double array of length newcol containing the upper
  bounds on the added columns.
  */
  void add_cols(int newcol, int newnz, const double *objx, const int *mstart,
                const int *mrwind, const double *dmatval, const double *bdl,
                const double *bdu) override {}

  /**
   * @brief Adds a name to a row or a column
   *
   * @param type   : 1 for row, 2 for column.
   * @param cnames : Character buffer containing the null-terminated string
   * names.
   * @param indice : index of the row or of the column.
   */
  void add_name(int type, const char *cnames, int indice) override {}

  /**
   * @brief Change coefficients in objective function
   *
   * @param mindex : indices of columns to modify
   * @param obj    : Values to set in objective function
   */
  void chg_obj(const std::vector<int> &mindex,
               const std::vector<double> &obj) override {}

  /**
   * @brief Change the problem's objective function sense to minimize or
   * maximize
   *
   * @param minimize : boolean that is true for minimize, false for maximization
   */
  void chg_obj_direction(const bool minimize) override {}

  /**
   * @brief Change bounds of some variables
   *
   * @param nbds   : number of bounds to change
   * @param mindex : indices of columns to modify
   * @param qbtype : types of the bounds to modify ('U' upper, 'L' lower, 'B'
   * both)
   * @param bnd    : new values for the bounds
   */
  void chg_bounds(const std::vector<int> &mindex,
                  const std::vector<char> &qbtype,
                  const std::vector<double> &bnd) override {}

  /**
   * @brief Change type of some columns
   *
   * @param nels   : number of elements to change
   * @param mindex : indices of columns to modify
   * @param qctype : New types of columns
   */
  void chg_col_type(const std::vector<int> &mindex,
                    const std::vector<char> &qctype) override {}

  /**
   * @brief Change rhs of a row
   *
   * @param id_row : index of the row
   * @param val    : new rhs value
   */
  void chg_rhs(int id_row, double val) override {}

  /**
   * @brief Change a coefficient in the matrix
   *
   * @param id_row : index of the row
   * @param id_col : index of the column
   * @param val    : new value to set in the matrix
   */
  void chg_coef(int id_row, int id_col, double val) override {}

  /**
   * @brief Change the name of a constraint
   *
   * @param id_row : index of the row
   * @param name   : new name of the row
   */
  void chg_row_name(int id_row, std::string const &name) override {}

  /**
   * @brief Change the name of a variable
   *
   * @param id_col : index of the column
   * @param name   : new name of the column
   */
  void chg_col_name(int id_col, std::string const &name) override {}

  /*************************************************************************************************
  -----------------------------    Methods to solve the problem
  ---------------------------------
  *************************************************************************************************/

  /**
   * @brief Solves a problem as LP
   *
   * @return lp_status      : status of the problem after resolution
   */
  int solve_lp() override {}

  /**
   * @brief Solves a problem as MIP
   *
   * @return lp_status      : status of the problem after resolution
   */
  int solve_mip() override { abc_.ReturnInt(); }

  /*************************************************************************************************
  -------------------------    Methods to get solutions information
  -----------------------------
  *************************************************************************************************/

  /**
  * @brief Returns the current basis into the user’s data arrays.
  *
  * @param rstatus    : Integer array of length ROWS to the basis status of the
  slack, surplus or artifficial variable associated with each row. The values
  depend on the solver used.May be NULL if not required.

  * @param cstatus    : Integer array of length COLS to hold the basis status of
  the columns in the constraint matrix. The values depend on the solver used.May
  be NULL if not required.
  */
  void get_basis(int *rstatus, int *cstatus) const override {}

  /**
   * @brief Get the optimal value of a MIP problem (available after method
   * "solve_mip")
   *
   * @return lb : optimal value of a MIP problem
   */
  double get_mip_value() const override { abc_.ReturnDouble(); }

  /**
   * @brief Get the optimal value of a LP problem (available after method
   * "solve_lp" )
   *
   * @return lb : optimal value of a LP problem
   */
  double get_lp_value() const override { abc_.ReturnDouble(); }

  /**
   * @brief Get the number of simplex iterations done in the last resolution of
   * the problem
   *
   * @return result : number of simplex iterations done in the last resolution
   * of the problem
   */
  int get_splex_num_of_ite_last() const override { abc_.ReturnInt(); }

  /**
   * @brief Get LP solution of a problem (available after method "solve_lp")
   *
   * @param primals    : values of primal variables
   * @param duals      : values of dual variables
   * @param reduced_cost: reduced_cost in an optimal basis
   */
  void get_lp_sol(double *primals, double *duals,
                  double *reduced_costs) override {}

  /**
   * @brief Get MIP solution of a problem (available after method "solve_mip")
   *
   * @param primals    : values of primal variables
   */
  void get_mip_sol(double *primals) override {}

  /*************************************************************************************************
  ------------------------    Methods to set algorithm or logs levels
  ---------------------------
  *************************************************************************************************/

  /**
   * @brief Sets log level of the solver
   *
   * @param loglevel : level of log to set
   */
  void set_output_log_level(int loglevel) override {}

  /**
   * @brief Sets algorithm used by solver to solve LP's
   *
   * @param algo : string of the name of the algorithm
   */
  void set_algorithm(std::string const &algo) override {}

  /**
   * @brief Sets the maximum number of threads used to perform optimization
   *
   * @param n_threads: maximum number of threads
   */
  void set_threads(int n_threads) override {}

  /**
   * @brief Sets the optimality gap
   *
   * @param gap: double of the value of wanted gap
   */
  void set_optimality_gap(double gap) override {}

  /**
   * @brief Sets the maximum number of simplex iterations the solver can perform
   *
   * @param iter: maximum number of simplex iterations
   */
  void set_simplex_iter(int iter) override {}
};