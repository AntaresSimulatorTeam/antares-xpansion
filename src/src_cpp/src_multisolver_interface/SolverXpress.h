#pragma once

#include "SolverAbstract.h"
#include "xprs.h"

/*!
* \class class SolverXpress
* \brief Daughter class of AsbtractSolver implementing solver XPRESS FICO
*/
class SolverXpress : public SolverAbstract {

/*************************************************************************************************
----------------------------------------    ATTRIBUTES    ---------------------------------------
*************************************************************************************************/
    static int _NumberOfProblems;	/*!< Counter of the total number of Cplex problems 
                                    declared to set or end the environment */
public:
	XPRSprob _xprs;					/*!< Problem in XPRESS */

/*************************************************************************************************
-----------------------------------    Constructor/Desctructor    --------------------------------
*************************************************************************************************/
public:
	/**
    * @brief Default constructor of a XPRESS solver
    */
	SolverXpress();

	/**
    * @brief Copy constructor of XPRESS, copy the problem toCopy in memory and name it "name"
	*
	* @param name 	: Name to give to the solver object (ony for CPLEX)
	* @param toCopy : Pointer to an AbstractSolver object, containing an XPRESS solver to copy
    */
	SolverXpress(const std::string& name, const SolverAbstract::Ptr toCopy);

	virtual ~SolverXpress();
	virtual int get_number_of_instances();

/*************************************************************************************************
---------------------------------    Output and stream management    -----------------------------
*************************************************************************************************/


/*************************************************************************************************
------    Destruction or creation of inner strctures and datas, closing environments    ----------
*************************************************************************************************/
public:
	virtual void init() ;
    virtual void free() ;

/*************************************************************************************************
-------------------------------    Reading & Writing problems    -------------------------------
*************************************************************************************************/
public:
	virtual void write_prob(const char* name, const char* flags) const;
    virtual void read_prob(const char* prob_name, const char* flags);
    virtual void copy_prob(const SolverAbstract::Ptr fictif_solv);


/*************************************************************************************************
-----------------------    Get general informations about problem    ----------------------------
*************************************************************************************************/
public:
    virtual int  get_ncols() const;
    virtual int  get_nrows() const;
	virtual int  get_nelems() const;
    virtual int get_n_integer_vars() const;
	virtual void get_obj(double* obj, int first, int last) const;
	virtual void get_rows(int* mstart, int* mclind, double* dmatval, int size, int* nels, 
                    int first, int last) const;
	virtual void get_row_type(char* qrtype, int first, int last) const;
	virtual void get_rhs(double* rhs, int first, int last) const;
	virtual void get_rhs_range(double* range, int first, int last) const;
	virtual void get_col_type(char* coltype, int first, int last) const;
	virtual int get_row_index(std::string const& name) const;
	virtual int get_col_index(std::string const& name) const;
	virtual void get_lb(double* lb, int fisrt, int last) const;
    virtual void get_ub(double* ub, int fisrt, int last) const;


/*************************************************************************************************
------------------------------    Methods to modify problem    ----------------------------------
*************************************************************************************************/
public:
    virtual void fix_first_stage(std::vector<double> const& x0);
    virtual void add_cut(std::vector<double> const& s, std::vector<double> const& x0, 
                    double rhs);
    virtual void del_rows(int first, int last);
    virtual void add_rows(int newrows, int newnz, const char* qrtype, const double* rhs, 
		            const double* range, const int* mstart, const int* mclind, 
                    const double* dmatval);
    virtual void add_cols(int newcol, int newnz, const double* objx, const int* mstart, 
                    const int* mrwind, const double* dmatval, const double* bdl, 
                    const double* bdu);
    virtual void add_name(int type, const char* cnames, int indice);
    virtual void chg_obj(int nels, const int* mindex, const double* obj);
    virtual void chg_bounds(int nbds, const int* mindex, const char* qbtype, const double* bnd);
    virtual void chg_col_type(int nels, const int* mindex, const char* qctype) const;
	virtual void chg_rhs(int id_row, double val);
    virtual void chg_coef(int id_row, int id_col, double val);
	
/*************************************************************************************************
-----------------------------    Methods to solve the problem    ---------------------------------
*************************************************************************************************/    
public:
    virtual void solve_lp(int& lp_status);
	virtual void solve_mip(int& lp_status);
	
/*************************************************************************************************
-------------------------    Methods to get solutions information    -----------------------------
*************************************************************************************************/
public:	
	virtual void get_basis(int* rstatus, int* cstatus) const;
	virtual void get_mip_value(double& lb) const;
	virtual void get_lp_value(double& lb) const;
	virtual void get_simplex_ite(int& result) const;
	virtual void get_LP_sol(double* primals, double* slacks, double* duals, 
                    double* reduced_costs);
    virtual void get_MIP_sol(double* primals, double* slacks);

/*************************************************************************************************
------------------------    Methods to set algorithm or logs levels    ---------------------------
*************************************************************************************************/
public:
	virtual void set_output_log_level(int loglevel);
	virtual void set_algorithm(std::string const& algo);
    virtual void set_threads(int n_threads);
	virtual void scaling(int scale);
	virtual void presolve(int presolve);
	virtual void optimality_gap(double gap);
	virtual void set_simplex_iter(int iter);
	virtual void numerical_emphasis(int val);
};

/************************************************************************************\
* Name:         optimizermsg                                                         *
* Purpose:      Display Optimizer error messages and warnings.                       *
* Arguments:    const char *sMsg    Message string                                   *
*               int nLen            Message length                                   *
*               int nMsgLvl         Message type                                     *
* Return Value: None.                                                                *
\************************************************************************************/
void errormsg(XPRSprob& xprs, const char* sSubName, int nLineNo, int nErrCode);

/************************************************************************************\
* Name:         errormsg                                                             *
* Purpose:      Display error information about failed subroutines.                  *
* Arguments:    const char *sSubName    Subroutine name                              *
*               int nLineNo             Line number                                  *
*               int nErrCode            Error code                                   *
* Return Value: None                                                                 *
\************************************************************************************/
void optimizermsg(XPRSprob prob, void* stream, const char* sMsg, int nLen, int nMsglvl);