#pragma once

#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "LogUtils.h"

class SolverLogManager {
 public:
  explicit SolverLogManager() = default;
  explicit SolverLogManager(const SolverLogManager &other)
      : SolverLogManager(other.log_file_path) {}

  explicit SolverLogManager(const std::filesystem::path &log_file)
      : log_file_path(log_file) {
    init();
  }

  SolverLogManager &operator=(const SolverLogManager &other) {
    if (this == &other) {
      return *this;
    }
    log_file_path = other.log_file_path;
    init();
    return *this;
  }

  void init() {
#ifdef __linux__
    if (log_file_path.empty() ||
        (log_file_ptr = fopen(log_file_path.string().c_str(), "a+")) == nullptr)
#elif _WIN32
    if (log_file_path.empty() ||
        (log_file_ptr = _fsopen(log_file_path.string().c_str(), "a+",
                                _SH_DENYNO)) == nullptr)
#endif
    {
      std::cout << "Invalid log file name passed as parameter: "
                << std::quoted(log_file_path.string()) << std::endl;
    } else {
      setvbuf(log_file_ptr, nullptr, _IONBF, 0);
    }
  }
  ~SolverLogManager() {
    if (log_file_ptr) {
      fclose(log_file_ptr);
      log_file_ptr = nullptr;
    }
  }

  FILE *log_file_ptr = nullptr;
  std::filesystem::path log_file_path = "";
};

class InvalidStatusException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidStatusException(int status, const std::string &action,
                         const std::string &log_message)
      : LogUtils::XpansionError<std::runtime_error>(
            "Failed to " + action + ": invalid status " +
                std::to_string(status) + " (0 expected)",
            log_message) {}
};

class InvalidRowSizeException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidRowSizeException(int expected_size, int actual_size,
                          const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            "Invalid row size for solver. " + std::to_string(actual_size) +
                " rows available (" + std::to_string(expected_size) +
                " expected)",
            log_location) {}
};

class InvalidColSizeException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidColSizeException(int expected_size, int actual_size,
                          const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            "Invalid col size for solver. " + std::to_string(actual_size) +
                " cols available (" + std::to_string(expected_size) +
                " expected)",
            log_location) {}
};

class InvalidBoundTypeException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidBoundTypeException(char qbtype, const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            std::string("Invalid bound type ") + qbtype + " for solver.",
            log_location) {}
};

class InvalidColTypeException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidColTypeException(char qctype, const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            std::string("Invalid col type ") + qctype + " for solver.",
            log_location) {}
};

class InvalidSolverOptionException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidSolverOptionException(const std::string &option,
                               const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            std::string("Invalid option '") + option + "' for solver.",
            log_location) {}
};

class InvalidSolverForCopyException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidSolverForCopyException(const std::string &from_solver,
                                const std::string &to_solver,
                                const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            "Can't copy " + from_solver + "solver from " + to_solver,
            log_location) {}
};

class InvalidSolverNameException
    : public LogUtils::XpansionError<std::runtime_error> {
 public:
  InvalidSolverNameException(const std::string &solver_name,
                             const std::string &log_location)
      : LogUtils::XpansionError<std::runtime_error>(
            "Solver '" + solver_name + "' not supported", log_location) {}
};
class GenericSolverException : public std::runtime_error {
 public:
  GenericSolverException(const std::string &message)
      : std::runtime_error(message) {}
};

class NotImplementedFeatureSolverException : public std::runtime_error {
 public:
  NotImplementedFeatureSolverException(const std::string &message)
      : std::runtime_error(message) {}
};

// Definition of optimality codes
enum SOLVER_STATUS {
  OPTIMAL,
  INFEASIBLE,
  UNBOUNDED,
  INForUNBOUND,
  UNKNOWN,
};

/*!
 * \class class SolverAbstract
 * \brief Virtual class to implement solvers methods
 */
class SolverAbstract {
 public:
  std::vector<std::string> SOLVER_STRING_STATUS = {
      "OPTIMAL", "INFEASIBLE", "UNBOUNDED", "INForUNBOUND", "UNKNOWN"};

  /*************************************************************************************************
  ----------------------------------------    ATTRIBUTES
  ---------------------------------------
  *************************************************************************************************/
 public:
  std::string _name;                           /*!< Name of the problem */
  typedef std::shared_ptr<SolverAbstract> Ptr; /*!< Ptr to the solver */
  std::list<std::ostream *>
      _streams; /*!< List of streams to print the output (default std::cout) */

  /*************************************************************************************************
  -----------------------------------    Constructor/Desctructor
  --------------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief constructor of SolverAbstract class : does nothing
   */
  SolverAbstract(){};

  /**
   * @brief Copy constructor, copy the problem "toCopy" in memory and name it
   * "name" if possible
   *
   * @param name 	: Name to give to new problem
   * @param toCopy : Pointer to an AbstractSolver object, containing a solver
   * object to copy
   */
  SolverAbstract(const std::string &name, const SolverAbstract::Ptr toCopy){};

  /**
   * @brief destructor of SolverAbstract class : does nothing
   */
  virtual ~SolverAbstract(){};

  /**
   * @brief Returns number of instances of solver currently in memory
   */
  virtual int get_number_of_instances() = 0;

  /**
   * @brief Returns the solver used
   */
  virtual std::string get_solver_name() const = 0;

  /*************************************************************************************************
  -----------------------------------    Output stream management
  ------------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief returns the list of streams used by the solver instance
   */
  std::list<std::ostream *> &get_stream() { return _streams; };
  FILE *_fp = nullptr;
  std::filesystem::path _log_file = "";
  /**
   * @brief add a stream to the list of streams used by the solver instance
   *
   * @param stream  : reference to a std::ostream object
   */
  void add_stream(std::ostream &stream) { get_stream().push_back(&stream); };
  void set_fp(FILE *fp) { _fp = fp; }
  /**
  * @brief Check if a status code is different to 0, throw
  InvalidStatusException if it occurs
  *
  * @param status         : status code to check
  * @param action_failed  : action which returned the non zero status code,
                          used to print the error message.
  */
  void zero_status_check(int status, const std::string &action_failed,
                         const std::string &log_location) const {
    if (status != 0) {
      throw InvalidStatusException(status, action_failed, log_location);
    }
  };

  /*************************************************************************************************
  ------    Destruction or creation of inner strctures and datas, closing
  environments    ----------
  *************************************************************************************************/
 public:
  /**
   * @brief Initializes a problem
   */
  virtual void init() = 0;

  /**
   * @brief Frees all the datas contained in the Solver environment
   */
  virtual void free() = 0;

  /*************************************************************************************************
  -------------------------------    Reading & Writing problems
  -------------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief writes an optimization problem in a MPS file
   *
   * @param name   : name of the file to write
   */
  virtual void write_prob_mps(const std::filesystem::path &filename) = 0;

  /**
   * @brief writes an optimization problem in a LP file
   *
   * @param name   : name of the file to write
   */
  virtual void write_prob_lp(const std::filesystem::path &filename) = 0;

  /**
   * @brief Writes the current basis to a file for later input into the
   * optimizer
   *
   * @param filename    : file name where the basis is written
   */
  virtual void write_basis(const std::filesystem::path &filename) = 0;

  /**
   * @brief Writes the current solution to a CSV format ASCII file, problem_name.asc (and .hdr).
   *
   * @param filename    : file name where the solution is written
   */
  virtual void write_sol(const std::filesystem::path &filename) = 0;

  /**
   * @brief reads an optimization problem contained in a MPS file
   *
   * @param name   : name of the file to read
   */
  virtual void read_prob_mps(const std::filesystem::path &filename) = 0;

  /**
   * @brief reads an optimization problem contained in a MPS file
   *
   * @param name   : name of the file to read
   */
  virtual void read_prob_lp(const std::filesystem::path &filename) = 0;

  /**
   * @brief Instructs the optimizer to read in a previously saved basis from a
   * file
   *
   * @param filename: File name where the basis is to be read
   */
  virtual void read_basis(const std::filesystem::path &filename) = 0;

  /**
   * @brief copy an existing problem
   *
   * @param prb_to_copy : Ptr to the
   */
  virtual void copy_prob(Ptr fictif_solv) = 0;

  /*************************************************************************************************
  -----------------------    Get general informations about problem
  ----------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief returns number of columns of the problem
   */
  virtual int get_ncols() const = 0;

  /**
   * @brief returns number of rows of the problem
   */
  virtual int get_nrows() const = 0;

  /**
   * @brief returns number of non zeros elements in the matrix, excluding
   * objective
   */
  virtual int get_nelems() const = 0;

  /**
   * @brief returns number of integer variables in the problem
   */
  virtual int get_n_integer_vars() const = 0;

  /**
   * @brief returns the objective function coefficients for the columns in a
   * given range
   */
  virtual void get_obj(double *obj, int first, int last) const = 0;

  /**
   * @brief Set the objective function coefficients to zero
   */
  virtual void set_obj_to_zero() = 0;

  /**
   * @brief Set the objective function coefficients for the columns in a
   * given range
   */
  virtual void set_obj(const double *obj, int first, int last) = 0;

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
  virtual void get_rows(int *mstart, int *mclind, double *dmatval, int size,
                        int *nels, int first, int last) const = 0;

  /**
  * @brief Returns the row types for the rows in a given range.
  *
  * @param qrtype : Character array of length last-first+1 characters where the
  row types will be returned (N, E, L, G, R)
  * @param first  : first index of row to get
  * @param last   : last index of row to get
  */
  virtual void get_row_type(char *qrtype, int first, int last) const = 0;

  /**
   * @brief Returns the right-hand sides of the rows in a given range.
   *
   * @param qrtype : Character array of length last-first+1 characters where the
   * rhs will be stored
   * @param first  : first index of row to get
   * @param last   : last index of row to get
   */
  virtual void get_rhs(double *rhs, int first, int last) const = 0;

  /**
  * @brief Returns the right hand side range values for the rows in a given
  range.
  *
  * @param range  : Double array of length last-first+1 where the right hand
  side range values are to be placed.
  * @param first  : first index of row to get
  * @param last   : last index of row to get
  */
  virtual void get_rhs_range(double *range, int first, int last) const = 0;

  /**
  * @brief Returns the column types for the columns in a given range.
  *
  * @param coltype    : Character array of length last-first+1 where the column
  types will be returned (C: continuous, I: integer, B: binary, S:
  semi-continuous, R: semi-continuous integer, P: partial integer)
  * @param first      : first index of row to get
  * @param last       : last index of row to get
  */
  virtual void get_col_type(char *coltype, int first, int last) const = 0;

  /**
   * @brief Returns the lower bounds for variables in a given range.
   *
   * @param lb     : Double array of length last-first+1 where the lower bounds
   * are to be placed.
   * @param first  : First column in the range.
   * @param last   : Last column in the range.
   */
  virtual void get_lb(double *lb, int fisrt, int last) const = 0;

  /**
   * @brief Returns the upper bounds for variables in a given range.
   *
   * @param lb     : Double array of length last-first+1 where the upper bounds
   * are to be placed.
   * @param first  : First column in the range.
   * @param last   : Last column in the range.
   */
  virtual void get_ub(double *ub, int fisrt, int last) const = 0;

  /**
   * @brief Returns the index of row named "name"
   *
   * @param name : name of row to get the index
   */
  virtual int get_row_index(std::string const &name) = 0;

  /**
   * @brief Returns the index of column named "name"
   *
   * @param name : name of column to get the index
   */
  virtual int get_col_index(std::string const &name) = 0;

  /**
   * @brief Returns the names of row from index first to last
   * cannot be declared as const because of some solver methods
   *
   * @param first : first index from which name has be returned
   * @param last  : last index from which name has be returned
   * @return names : vector of names
   */
  virtual std::vector<std::string> get_row_names(int first, int last) = 0;

  /**
   * @brief Returns the names of rows
   * @return names : vector of names
   */
  virtual std::vector<std::string> get_row_names() = 0;

  /**
   * @brief Returns the names of columns from index first to last
   * cannot be declared as const because of some solver methods
   *
   * @param first : first index from which name has be returned
   * @param last  : last index from which name has be returned
   * @return names : vector of names
   */
  virtual std::vector<std::string> get_col_names(int first, int last) = 0;

  /**
   * @brief Returns the names of columns
   * @return names : vector of names
   */
  virtual std::vector<std::string> get_col_names() = 0;

  /*************************************************************************************************
  ------------------------------    Methods to modify problem
  ----------------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief Deletes rows between index first and last
   *
   * @param first  : first row index to delete
   * @param last   : last row index to delete
   */
  virtual void del_rows(int first, int last) = 0;

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
  virtual void add_rows(int newrows, int newnz, const char *qrtype,
                        const double *rhs, const double *range,
                        const int *mstart, const int *mclind,
                        const double *dmatval,
                        const std::vector<std::string> &names = {}) = 0;

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
  virtual void add_cols(int newcol, int newnz, const double *objx,
                        const int *mstart, const int *mrwind,
                        const double *dmatval, const double *bdl,
                        const double *bdu) = 0;

  /**
   * @brief Adds a name to a row or a column
   *
   * @param type   : 1 for row, 2 for column.
   * @param cnames : Character buffer containing the null-terminated string
   * names.
   * @param indice : index of the row or of the column.
   */
  virtual void add_name(int type, const char *cnames, int indice) = 0;
  virtual void add_names(int type, const std::vector<std::string> &cnames,
                         int first, int end) = 0;

  /**
   * @brief Change coefficients in objective function
   *
   * @param mindex : indices of columns to modify
   * @param obj    : Values to set in objective function
   */
  virtual void chg_obj(const std::vector<int> &mindex,
                       const std::vector<double> &obj) = 0;

  /**
   * @brief Change the problem's objective function sense to minimize or
   * maximize
   *
   * @param minimize : boolean that is true for minimize, false for maximization
   */
  virtual void chg_obj_direction(const bool minimize) = 0;

  /**
   * @brief Change bounds of some variables
   *
   * @param nbds   : number of bounds to change
   * @param mindex : indices of columns to modify
   * @param qbtype : types of the bounds to modify ('U' upper, 'L' lower, 'B'
   * both)
   * @param bnd    : new values for the bounds
   */
  virtual void chg_bounds(const std::vector<int> &mindex,
                          const std::vector<char> &qbtype,
                          const std::vector<double> &bnd) = 0;

  /**
   * @brief Change type of some columns
   *
   * @param nels   : number of elements to change
   * @param mindex : indices of columns to modify
   * @param qctype : New types of columns
   */
  virtual void chg_col_type(const std::vector<int> &mindex,
                            const std::vector<char> &qctype) = 0;

  /**
   * @brief Change rhs of a row
   *
   * @param id_row : index of the row
   * @param val    : new rhs value
   */
  virtual void chg_rhs(int id_row, double val) = 0;

  /**
   * @brief Change a coefficient in the matrix
   *
   * @param id_row : index of the row
   * @param id_col : index of the column
   * @param val    : new value to set in the matrix
   */
  virtual void chg_coef(int id_row, int id_col, double val) = 0;

  /**
   * @brief Change the name of a constraint
   *
   * @param id_row : index of the row
   * @param name   : new name of the row
   */
  virtual void chg_row_name(int id_row, std::string const &name) = 0;

  /**
   * @brief Change the name of a variable
   *
   * @param id_col : index of the column
   * @param name   : new name of the column
   */
  virtual void chg_col_name(int id_col, std::string const &name) = 0;

  /*************************************************************************************************
  -----------------------------    Methods to solve the problem
  ---------------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief Solves a problem as LP
   *
   * @return lp_status      : status of the problem after resolution
   */
  virtual int solve_lp() = 0;

  /**
   * @brief Solves a problem as MIP
   *
   * @return lp_status      : status of the problem after resolution
   */
  virtual int solve_mip() = 0;

  /*************************************************************************************************
  -------------------------    Methods to get solutions information
  -----------------------------
  *************************************************************************************************/
 public:
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
  virtual void get_basis(int *rstatus, int *cstatus) const = 0;

  /**
   * @brief Get the optimal value of a MIP problem (available after method
   * "solve_mip")
   *
   * @return lb : optimal value of a MIP problem
   */
  virtual double get_mip_value() const = 0;

  /**
   * @brief Get the optimal value of a LP problem (available after method
   * "solve_lp" )
   *
   * @return lb : optimal value of a LP problem
   */
  virtual double get_lp_value() const = 0;

  /**
   * @brief Get the number of simplex iterations done in the last resolution of
   * the problem
   *
   * @return result : number of simplex iterations done in the last resolution
   * of the problem
   */
  virtual int get_splex_num_of_ite_last() const = 0;

  /**
   * @brief Get LP solution of a problem (available after method "solve_lp")
   *
   * @param primals    : values of primal variables
   * @param duals      : values of dual variables
   * @param reduced_cost: reduced_cost in an optimal basis
   */
  virtual void get_lp_sol(double *primals, double *duals,
                          double *reduced_costs) = 0;

  /**
   * @brief Get MIP solution of a problem (available after method "solve_mip")
   *
   * @param primals    : values of primal variables
   */
  virtual void get_mip_sol(double *primals) = 0;

  /*************************************************************************************************
  ------------------------    Methods to set algorithm or logs levels
  ---------------------------
  *************************************************************************************************/
 public:
  /**
   * @brief Sets log level of the solver
   *
   * @param loglevel : level of log to set
   */
  virtual void set_output_log_level(int loglevel) = 0;

  /**
   * @brief Sets algorithm used by solver to solve LP's
   *
   * @param algo : string of the name of the algorithm
   */
  virtual void set_algorithm(std::string const &algo) = 0;

  /**
   * @brief Sets the maximum number of threads used to perform optimization
   *
   * @param n_threads: maximum number of threads
   */
  virtual void set_threads(int n_threads) = 0;

  /**
   * @brief Sets the optimality gap
   *
   * @param gap: double of the value of wanted gap
   */
  virtual void set_optimality_gap(double gap) = 0;

  /**
   * @brief Sets the maximum number of simplex iterations the solver can perform
   *
   * @param iter: maximum number of simplex iterations
   */
  virtual void set_simplex_iter(int iter) = 0;
};
