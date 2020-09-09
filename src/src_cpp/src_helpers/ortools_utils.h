#pragma once

#include <sstream>

#include "ortools/linear_solver/linear_solver.h"


#if ( (!defined(ORTOOLS_LP_SOLVER_TYPE)) || (!defined(ORTOOLS_MIP_SOLVER_TYPE)) )
#if defined(CPLEX_SOLVER)
#define ORTOOLS_LP_SOLVER_TYPE operations_research::MPSolver::CPLEX_LINEAR_PROGRAMMING
#define ORTOOLS_MIP_SOLVER_TYPE operations_research::MPSolver::CPLEX_MIXED_INTEGER_PROGRAMMING
#elif defined(XPRESS_SOLVER)
#define ORTOOLS_LP_SOLVER_TYPE operations_research::MPSolver::XPRESS_LINEAR_PROGRAMMING
#define ORTOOLS_MIP_SOLVER_TYPE operations_research::MPSolver::XPRESS_MIXED_INTEGER_PROGRAMMING
#elif defined(GUROBI_SOLVER)
#define ORTOOLS_LP_SOLVER_TYPE operations_research::MPSolver::GUROBI_LINEAR_PROGRAMMING
#define ORTOOLS_MIP_SOLVER_TYPE operations_research::MPSolver::GUROBI_MIXED_INTEGER_PROGRAMMING
#else
#define ORTOOLS_LP_SOLVER_TYPE operations_research::MPSolver::CLP_LINEAR_PROGRAMMING
#define ORTOOLS_MIP_SOLVER_TYPE operations_research::MPSolver::CBC_MIXED_INTEGER_PROGRAMMING
#endif
#endif


/**
 * @brief reads problem from an mps file
 *
 * @param solver_p  : solver into which the model will be loaded
 * @param filename_p  : path to the mps file
 */
operations_research::MPSolverResponseStatus ORTreadmps(operations_research::MPSolver & solver_p, std::string const & filename_p);

/**
 * @brief writes a problem into an mps format
 *
 * @param solver_p  : solver containing the model to export
 * @param filename_p  : path to the mps file to produce
 */
bool ORTwritemps(operations_research::MPSolver const & solver_p, std::string const & filename_p);

/**
 * @brief writes a problem into an lp format
 *
 * @param solver_p  : solver containing the model to export
 * @param filename_p  : path to the lp file to produce
 */
bool ORTwritelp(operations_research::MPSolver const & solver_p, std::string const & filename_p);


/**
 * @brief enables to print the problem in a human readable format
 *
 * @param solver_p  : solver containing the model to consider.
 * @param oss_p  : string stream that will contain the output
 * @param index_p : optional parameter that defaults to false,
 *              if set to true the indices of variables will be used (prefixed by x)
 *              if set to false the names of variables will be used
 */
void ORTdescribe(operations_research::MPSolver const & solver_p, std::ostringstream & oss_p, bool index_p = false);

/**
 * @brief Returns the nonzeros in the constraint matrix for the rows in a given range
 *
 * @param solver_p  : solver containing the model to consider.
 * @param mstart_p  : will be filled with the indices indicating the starting offsets in the
 *      mclind_p and dmatval_p arrays for each requested row.
 *      Column i starts at position mstart_p[i] in the mrwind_p and dmatval_p arrays, and has
 *      mstart_p[i+1]-mstart_p[i] elements in it.
 * @param mclind_p  : will be filled with the column indices of the nonzero elements for each row.
 * @param dmatval_p  : will be filled with the nonzero element values.
 * @param first_p  : First row in the range.
 * @param last_p  : Last row in the range.
 */
void ORTgetrows(operations_research::MPSolver const & solver_p,
                 std::vector<int> & mstart_p, std::vector<int> & mclind_p, std::vector<double> & dmatval_p,
                 int first_p, int last_p);

/**
 * @brief Change the objective function coefficients of an existent problem
 *
 * @param solver_p  : The problem to modify.
 * @param mindex_p  : Integer array of the indices of the columns on which the coefficients will change.
 *      An index of -1 indicates that the fixed part of the objective function should change.
 * @param obj_p  : new objective coefficients corresponding to the variables with indices in mindex_p.
 */
void ORTchgobj(operations_research::MPSolver & solver_p, std::vector<int> const & mindex_p, std::vector<double> const & obj_p);

/**
 * @brief Returns the objective function coefficients for the columns in a given range.
 *
 * @param solver_p  : solver containing the model to consider.
 * @param obj_p  :  will be filled with the objective function coefficients
 * @param first_p  : first column to consider.
 * @param last_p  : last column to consider.
 */
void ORTgetobj(operations_research::MPSolver const & solver_p, std::vector<double> & obj_p, int first_p, int last_p);

/**
 * @brief Adds variables to an existent solver
 *
 * @param solver_p  : solver containing the model to modify.
 * @param objx_p : array containing the objective function coefficients of the new columns.
 * @param mstart_p : Integer array containing the offsets in the mrwind_p and dmatval_p arrays of
 *          the start of the elements for each column.
 * @param mrwind_p : Integer array containing the row indices for the elements in each column.
 * @param dmatval_p : Double array containing the element values.
 * @param bdl_p : Double array containing the lower bounds on the added columns.
 * @param bdu_p : Double array containing the upper bounds on the added columns.
 * @param colTypes_p : array indicating the types of columns to add :
 *          'B' for binary variables
 *          'I' for integer variables
 *          'C' for continuous variables
 * @param colNames_p : optional parameter. array containing the names of the new columns to add.
 */
void ORTaddcols(operations_research::MPSolver & solver_p,
                std::vector<double> const & objx_p, 
                std::vector<int> const & mstart_p, std::vector<int> const & mrwind_p, std::vector<double> const & dmatval_p,
                std::vector<double> const & bdl_p, std::vector<double> const & bdu_p,
                std::vector<char> const & colTypes_p,
                std::vector<std::string> const & colNames_p = {});

/**
 * @brief Adds constraints to an existent solver
 *
 * @param solver_p  : solver containing the model to modify.
 * @param qrtype_p : Character array containing the row types:
 *         L : indicates a <= row;
 *         G : indicates a >= row;
 *         E : indicates an = row.
 *         R : indicates a range constraint;
 *         N : indicates a nonbinding constraint (these will be ignored).
 * @param rhs_p : Double array of containing the right hand side elements.
 *         This is the UB for L and R constraints and the LB for G constraints.
 * @param range_p : Double array containing the row range elements. The values in the range array will only be read for R type rows.
 *          for R rows we assume that the ub is the rhs and the LB is rhs-range
 * @param mstart_p : Integer array of containing the offsets in the mclind_p and dmatval_p arrays of the start of the elements for each row.
 * @param mclind_p : Integer array of containing the (contiguous) column indices for the elements in each row.
 * @param dmatval_p : Double array of containing the (contiguous) element values.
 *
 * @note ignores non-binding rows
 */
void ORTaddrows(operations_research::MPSolver & solver_p,
                std::vector<char> const &  qrtype_p,
                std::vector<double>  const & rhs_p,
                std::vector<double>  const & range_p,
                std::vector<int> const & mstart_p, std::vector<int> const & mclind_p, std::vector<double> const & dmatval_p);

/**
 * @brief returns the solution of a solved problem
 *
 * @param solver_p  : solver containing the solved model.
 * @param x_p : will be filled with the variables values of the retrieved solution
 */
void ORTgetlpsolution(operations_research::MPSolver const & solver_p, std::vector<double> & x_p);

/**
 * @brief returns the dual values of a solved problem
 *
 * @param solver_p  : solver containing the solved model.
 * @param dual_p : will be filled with the dual values of the retrieved solution
 */
void ORTgetlpdual(operations_research::MPSolver const & solver_p, std::vector<double> & dual_p);

/**
 * @brief Returns the reduced costs of a solved problem
 *
 * @param solver_p  : solver containing the solved model.
 * @param dj_p : will be filled with the reduced costs
 */
void ORTgetlpreducedcost(operations_research::MPSolver const & solver_p, std::vector<double> & dj_p);

/**
 * @brief Returns the row types for the rows in a given range.
 *
 * @param solver_p  : solver containing the model.
 * @param qrtype_p : will be filled with row types:
 *      N : indicates a free constraint;
 *      L : indicates a <= constraint;
 *      E : indicates an = constraint;
 *      G : indicates a >= constraint;
 *      R : indicates a range constraint.
 * @param first_p : First row in the range.
 * @param last_p : Last row in the range
 */
void ORTgetrowtype(operations_research::MPSolver const & solver_p, std::vector<char> & qrtype_p, int first_p, int last_p);

/**
 * @brief Returns the right hand side elements for the rows in a given range.
 *
 * @param solver_p  : solver containing the model.
 * @param rhs_p : will be filled with the right hand side elements.
 *      This is the lb for >= constraints, the ub for <= and R constraints.
 * @param first_p : First row in the range.
 * @param last_p : Last row in the range
 *
 * @note if the constraint is a range, returns the ub.
 * @note if the constraint is unbound, still returns the ub i.e. solver's +inf.
 */
void ORTgetrhs(operations_research::MPSolver const & solver_p, std::vector<double> & rhs_p, int first_p, int last_p);

/**
 * @brief Returns the right hand side range values for the rows in a given range.
 *
 * @param solver_p  : solver containing the model.
 * @param range_p : will be filled with the right hand side range values (ie ub-lb or infinity).
 * @param first_p : First row in the range.
 * @param last_p : Last row in the range
 */
void ORTgetrhsrange(operations_research::MPSolver const & solver_p, std::vector<double> &  range_p, int first_p, int last_p);

/**
 * @brief Returns infos about the columns
 *
 * @param solver_p  : solver containing the model.
 * @param coltype_p : will be filled with the types of the columns in the specified range:
 *          'B' for binary variables
 *          'I' for integer variables
 *          'C' for continuous variables
 * @param bdl_p : will be filled with the lower bounds of the columns in the specified range:
 * @param bdu_p : will be filled with the upper bounds of the columns in the specified range:
 * @param first_p : First column in the range.
 * @param last_p : Last column in the range
 *
 * @note return 'I' if the variable is binary or integer and it was fixed
 */
void ORTgetcolinfo(operations_research::MPSolver const & solver_p,
                   std::vector<char> & coltype_p,
                   std::vector<double> & bdl_p, std::vector<double> & bdu_p,
                   int first_p, int last_p);

/**
 * @brief Deactivates some rows in an existent model
 *
 * @param solver_p  : solver containing the model to modify.
 * @param mindex : indices of the rows to deactivate.
 *
 * @note note that this method does not delete the rows but simply unbounds the constraint and supresses its terms
 * i.e. replaces the indexed rows with  -inf <= 0 <= +inf
 */
void ORTdeactivaterows(operations_research::MPSolver & solver_p, std::vector<int> const & mindex);

/**
 * @brief Returns the current basis
 *
 * @param solver_p  : solver containing the model to modify.
 * @param rstatus_p : will be filled with  the basis status of the slack, surplus or artificial variable associated with each row. The status will be one of:
 *      0 : slack, surplus or artificial is non-basic at lower bound;
 *      1 : slack, surplus or artificial is basic;
 *      2 : slack or surplus is non-basic at upper bound.
 *      3 : slack or surplus is super-basic.
May be NULL if not required.
 * @param cstatus_p : will be filled with the basis status of the columns in the constraint matrix. The status will be one of:
 *      0 : variable is non-basic at lower bound, or superbasic at zero if the variable has no lower bound;
 *      1 : variable is basic;
 *      2 : variable is non-basic at upper bound;
 *      3 : variable is super-basic.
May be NULL if not required.
 */
void ORTgetbasis(operations_research::MPSolver & solver_p, std::vector<int> & rstatus_p, std::vector<int> & cstatus_p);

/**
 * @brief Returns the current basis
 *
 * @param solver_p  : solver containing the model to modify.
 * @param mindex_p : int array containing the indices of the columns on which the bounds will change.
 * @param qbtype_p : Character array of same length as mindex_p indicating the type of bound to change:
 *      U : indicates change the upper bound;
 *      L : indicates change the lower bound;
 *      B : indicates change both bounds, i.e. fix the column.
 * @param bnd_p : Double array of same length as mindex_p giving the new bound values.
 *
 * @note if the same bound is changed twice the bound at the end of the vector will be used and no warning will be issued
 */
void ORTchgbounds(operations_research::MPSolver & solver_p,
                  std::vector<int> const & mindex_p,
                  std::vector<char> const & qbtype_p,
                  std::vector<double> const & bnd_p);

/**
 * @brief Returns the current basis
 *
 * @param outSolver_p  : solver containing the model with renamed variables.
 * @param inSolver_p  : solver containing the model to copy.
 * @param names_p : int array containing the indices of the columns on which the bounds will change.
 *
 * @note care when copying between solvers of different types : no special verifications are done (eg. infinity values correspondance)
 * @note duplicate/empty names will be named automatically by ortools
 */
void ORTcopyandrenamevars(operations_research::MPSolver & outSolver_p,
                            operations_research::MPSolver const & inSolver_p,
                            std::vector<std::string> const & names_p);
