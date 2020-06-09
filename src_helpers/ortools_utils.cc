#include <fstream>

#include "ortools_utils.h"
#include "ortools/lp_data/mps_reader.h"
#include "ortools/linear_solver/linear_solver.pb.h"
#include "ortools/linear_solver/model_exporter.h"

//TODO remove the nbXXXX variables : they were here to manage arrays => not needed with vectors

operations_research::MPSolverResponseStatus ORTreadmps(operations_research::MPSolver & solver_p, std::string const & filename_p)
{
    operations_research::MPModelProto model_proto_l;
    std::ifstream mpsfile(filename_p.c_str());
    if(mpsfile.good())
    {
        operations_research::glop::MPSReader().ParseFile(filename_p, &model_proto_l);
        std::string errorMessage_l;
        const operations_research::MPSolverResponseStatus status = solver_p.LoadModelFromProtoWithUniqueNamesOrDie(model_proto_l, &errorMessage_l);
        if(errorMessage_l.length())
        {
            std::cerr << "readMPS::error message: " << errorMessage_l << std::endl;
        }
        return status;
    }
    else
    {
        std::cerr << "MPS file " << filename_p << " was not found!";
    }
    return operations_research::MPSOLVER_MODEL_INVALID;
}

bool ORTwritemps(operations_research::MPSolver const & solver_p, std::string const & filename_p)
{
    operations_research::MPModelProto proto_l;
	solver_p.ExportModelToProto(&proto_l);
	operations_research::MPModelExportOptions options_l;
	const auto status_l = operations_research::ExportModelAsMpsFormat(proto_l, options_l);

    std::ofstream mpsOut(filename_p);
    mpsOut <<  status_l.value_or("");
    mpsOut.close();
}

void ORTdescribe(operations_research::MPSolver const & solver_p, std::ostringstream & oss_p, bool index_p)
{
    operations_research::MPObjective const & objective_l(solver_p.Objective());
    oss_p << ( objective_l.maximization() ? "max" : "min" ) << "\t" << objective_l.offset() << "\t" ;
    for(auto pairVarCoeff : objective_l.terms())
    {
        oss_p << pairVarCoeff.second << " " << ( index_p ? "x"+std::to_string(pairVarCoeff.first->index()) : pairVarCoeff.first->name() )<< "\t";
    }

    for(auto constraint : solver_p.constraints())
    {
        oss_p << std::endl << constraint->name() << ":\t" << constraint->lb() << "\t<=\t";
        for(auto pairVarCoeff : constraint->terms())
        {
            oss_p << pairVarCoeff.second << " " << ( index_p ? "x"+std::to_string(pairVarCoeff.first->index()) : pairVarCoeff.first->name() ) << "\t";
        }
        oss_p << "<=\t" << constraint->ub();
    }

     oss_p << std::endl;
}


void ORTgetrows(operations_research::MPSolver const & solver_p, std::vector<int> & mstart_p, std::vector<int> & mclind_p, std::vector<double> & dmatval_p, int first_p, int last_p)
{
    mstart_p.clear();
    mclind_p.clear();
    dmatval_p.clear();

    int ind(0);
    for(auto itConstraint_l(solver_p.constraints().cbegin()+first_p), itConstraintEnd_l(solver_p.constraints().cbegin()+last_p+1) ; itConstraint_l!=itConstraintEnd_l ; ++itConstraint_l)
    {
        operations_research::MPConstraint* constraint_l(*itConstraint_l);
        mstart_p.push_back(ind);
        for(auto termVarVal : constraint_l->terms())
        {
            mclind_p.push_back(termVarVal.first->index());
            dmatval_p.push_back(termVarVal.second);
            ++ind;
        }
    }
}



void ORTchgobj(operations_research::MPSolver & solver_p, std::vector<int> const & mindex_p, std::vector<double> const & obj_p)
{
	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();

	operations_research::MPObjective * objective_l(solver_p.MutableObjective());
	for(int cnt_l(0); cnt_l < mindex_p.size(); ++cnt_l)
	{
		if ( -1 == mindex_p[cnt_l] )
		{
			objective_l->SetOffset(obj_p[cnt_l]);
		}
		else
		{
			objective_l->SetCoefficient(variables_l[mindex_p[cnt_l]], obj_p[cnt_l]);
		}
	}
}

void ORTgetobj(operations_research::MPSolver const & solver_p, std::vector<double> & obj_p, int first_p, int last_p)
{
    obj_p.clear();

	operations_research::MPObjective const & objective_l(solver_p.Objective());
	auto mapVarCoeff = objective_l.terms();
	for(auto itVariable_l(solver_p.variables().cbegin()+first_p), itVariableEnd_l(solver_p.variables().cbegin()+last_p+1) ; itVariable_l!=itVariableEnd_l ; ++itVariable_l)
	{
		auto it_l = mapVarCoeff.find(*itVariable_l);
		if ( it_l != mapVarCoeff.end() )
		{
			obj_p.push_back(it_l->second);
		}
		else
		{
			obj_p.push_back(0);
		}
	}
}

void ORTaddcols(operations_research::MPSolver & solver_p, std::vector<double> const & objx_p, std::vector<int> const & mstart_p, std::vector<int> const & mrwind_p, std::vector<double> const & dmatval_p, std::vector<double> const & bdl_p, std::vector<double> const & bdu_p, std::vector<char> const & colTypes_p, std::vector<std::string> const & colNames_p)
{
	assert(objx_p.size() == mstart_p.size());
    assert(mrwind_p.size() == dmatval_p.size());

	operations_research::MPObjective* objective_l = solver_p.MutableObjective();
	const std::vector<operations_research::MPConstraint*> & constraints_l = solver_p.constraints();

	for(int col_l(0); col_l < objx_p.size(); ++col_l)
	{
		const std::string& name_l = (colNames_p.size() == objx_p.size()) ? colNames_p[col_l] : "addedCol_" + std::to_string(col_l);
		operations_research::MPVariable* mpVar_l;
		switch ( colTypes_p[col_l] )
		{
			case 'C':
			{
     			mpVar_l = solver_p.MakeNumVar(bdl_p[col_l], bdu_p[col_l] , name_l);
                break;
			}
			case 'I':
			{
     			mpVar_l = solver_p.MakeIntVar(bdl_p[col_l], bdu_p[col_l] , name_l);
                break;
			}
			case 'B':
			{
     			mpVar_l = solver_p.MakeBoolVar(name_l);
                break;
			}
			default:
			{
				std::cerr << "type of the variable " << name_l << " is not handled : -" << colTypes_p[col_l] << "-!\n" ;
			}
		}

		objective_l->SetCoefficient(mpVar_l, objx_p[col_l]);

		int startIndex_l = (mstart_p.size() > col_l) ? mstart_p[col_l] : 0;
		int endIndex_l(0);
		if(0 == mstart_p.size())
		{
			endIndex_l = 0;
		}
		else if(col_l == mstart_p.size()-1)
		{
			endIndex_l = mrwind_p.size();
		}
		else
		{
			endIndex_l = mstart_p[col_l+1];
		}
		for(int ind_l(startIndex_l); ind_l < endIndex_l ; ++ind_l)
		{
			constraints_l[mrwind_p[ind_l]]->SetCoefficient(mpVar_l, dmatval_p[ind_l]);
		}

	}
}

void ORTaddrows(operations_research::MPSolver & solver_p, std::vector<char> const &  qrtype_p, std::vector<double>  const & rhs_p, std::vector<double>  const & range_p, std::vector<int> const & mstart_p, std::vector<int> const & mclind_p, std::vector<double> const & dmatval_p)
{
	assert(qrtype_p.size() == rhs_p.size());
	assert((mstart_p.size() == 0 ) || (mstart_p.size() == qrtype_p.size()) );
	assert((range_p.size() == 0 ) || (range_p.size() == qrtype_p.size()) );
	assert(mclind_p.size() == dmatval_p.size());

	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();

	for(int row_l(0); row_l < qrtype_p.size(); ++row_l)
	{
		double lb_l(-solver_p.infinity());
		double ub_l(solver_p.infinity());
		const std::string& name_l = "addedRow_" + std::to_string(row_l);

		switch ( qrtype_p[row_l] )
		{
			case 'L':
			{
				ub_l = rhs_p[row_l];
                break;
			}
			case 'G':
			{
				lb_l = rhs_p[row_l];
                break;
			}
			case 'E':
			{
				lb_l = rhs_p[row_l];
				ub_l = rhs_p[row_l];
                break;
			}
			case 'R':
			{
                if(range_p[row_l] >= 0 )
                {
                    ub_l = rhs_p[row_l];
                    lb_l = rhs_p[row_l] - range_p[row_l];
                }
                else
                {
                    std::cerr << "negative range values are not handled!";
                }
                break;
			}
			case 'N':
			{
				continue;//ignore non-binding rows
                break;
			}
			default:
			{
				std::cerr << "type of the row " << name_l << " is not handled : " << qrtype_p[row_l] << "!\n" ;
			}
		}

		operations_research::MPConstraint* const mpConstraint_l = solver_p.MakeRowConstraint(lb_l, ub_l , name_l);
		int startIndex_l = (mstart_p.size() > row_l) ? mstart_p[row_l] : 0;
		int endIndex_l(0);
		if(0 == mstart_p.size())
		{
			endIndex_l = 0;
		}
		else if(row_l == mstart_p.size()-1)
		{
			endIndex_l = mclind_p.size();
		}
		else
		{
			endIndex_l = mstart_p[row_l+1];
		}
		for(int ind_l(startIndex_l); ind_l < endIndex_l ; ++ind_l)
		{
			mpConstraint_l->SetCoefficient(variables_l[mclind_p[ind_l]], dmatval_p[ind_l]);
		}
	}
}

// void ORTgetlpsol(operations_research::MPSolver const & solver_p, std::vector<double> & x_p, std::vector<double> & dual_p, std::vector<double> & dj_p)
// {
//     x_p.clear();
//     dual_p.clear();
//     dj_p.clear();

// 	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
// 	for(operations_research::MPVariable* var_l : solver_p.variables())
// 	{
// 		x_p.push_back(var_l->solution_value());
// 		dj_p.push_back(var_l->reduced_cost());
// 	}
// 	for(operations_research::MPConstraint* constraint_l : solver_p.constraints())
// 	{
// 		dual_p.push_back(constraint_l->dual_value());
// 	}

// }

void ORTgetlpsolution(operations_research::MPSolver const & solver_p, std::vector<double> & x_p)
{
    x_p.clear();

	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
	for(operations_research::MPVariable* var_l : solver_p.variables())
	{
		x_p.push_back(var_l->solution_value());
	}
}

void ORTgetlpdual(operations_research::MPSolver const & solver_p, std::vector<double> & dual_p)
{
    dual_p.clear();

	for(operations_research::MPConstraint* constraint_l : solver_p.constraints())
	{
		dual_p.push_back(constraint_l->dual_value());
	}
}

void ORTgetlpreducedcost(operations_research::MPSolver const & solver_p, std::vector<double> & dj_p)
{
    dj_p.clear();

	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
	for(operations_research::MPVariable* var_l : solver_p.variables())
	{
		dj_p.push_back(var_l->reduced_cost());
	}
}

void ORTgetrowtype(operations_research::MPSolver const & solver_p, std::vector<char> & qrtype_p, int first_p, int last_p)
{
    qrtype_p.clear();

    for(auto itConstraint_l(solver_p.constraints().cbegin()+first_p), itConstraintEnd_l(solver_p.constraints().cbegin()+last_p+1) ; itConstraint_l!=itConstraintEnd_l ; ++itConstraint_l)
    {
        operations_research::MPConstraint* constraint_l(*itConstraint_l);

        if( (constraint_l->lb() == -solver_p.infinity()) && (constraint_l->ub() == solver_p.infinity()) )
        {
            qrtype_p.push_back('N');
        }
        if( constraint_l->lb() == -solver_p.infinity() )
        {
            qrtype_p.push_back('L');
        }
        else if ( constraint_l->ub() == solver_p.infinity())
        {
            qrtype_p.push_back('G');
        }
        else if ( constraint_l->lb() == constraint_l->ub() )
        {
            qrtype_p.push_back('E');
        }
        else
        {
            //TODO : we assume that the RHS for ranges is the ub : verify consistency with xpress results
            qrtype_p.push_back('R');
        }
    }
}

void ORTgetrhs(operations_research::MPSolver const & solver_p, std::vector<double> & rhs_p, int first_p, int last_p)
{
    rhs_p.clear();

    for(auto itConstraint_l(solver_p.constraints().cbegin()+first_p), itConstraintEnd_l(solver_p.constraints().cbegin()+last_p+1) ; itConstraint_l!=itConstraintEnd_l ; ++itConstraint_l)
    {
        operations_research::MPConstraint* constraint_l(*itConstraint_l);

        if( constraint_l->lb() == -solver_p.infinity() )
        {
            rhs_p.push_back(constraint_l->ub());
        }
        else if ( constraint_l->ub() == solver_p.infinity())
        {
            rhs_p.push_back(constraint_l->lb());
        }
        else if ( constraint_l->lb() == constraint_l->ub() )
        {
            rhs_p.push_back(constraint_l->lb());
        }
        else
        {
            //TODO : we assume that the RHS for ranges is the ub : verify consistency with xpress results
            rhs_p.push_back(constraint_l->ub());
        }
    }
}

void ORTgetrhsrange(operations_research::MPSolver const & solver_p, std::vector<double> &  range_p, int first_p, int last_p)
{
    range_p.clear();

    for(auto itConstraint_l(solver_p.constraints().cbegin()+first_p), itConstraintEnd_l(solver_p.constraints().cbegin()+last_p+1) ; itConstraint_l!=itConstraintEnd_l ; ++itConstraint_l)
    {
        operations_research::MPConstraint* constraint_l(*itConstraint_l);

        if( (constraint_l->lb() == -solver_p.infinity()) ||  ( constraint_l->ub() == solver_p.infinity()) )
        {
            range_p.push_back( solver_p.infinity() );
        }
        else if ( constraint_l->ub() == solver_p.infinity())
        {
            range_p.push_back( 0 );
        }
        else
        {
            range_p.push_back(constraint_l->ub() - constraint_l->lb());
        }
    }
}

void ORTgetcolinfo(operations_research::MPSolver const & solver_p, std::vector<char> & coltype_p, std::vector<double> & bdl_p, std::vector<double> & bdu_p, int first_p, int last_p)
{
    bdl_p.clear();
    bdu_p.clear();

    for(auto itVariable_l(solver_p.variables().cbegin()+first_p), itVariableEnd_l(solver_p.variables().cbegin()+last_p+1) ; itVariable_l!=itVariableEnd_l ; ++itVariable_l)
    {
        operations_research::MPVariable* variable_l(*itVariable_l);

        bdl_p.push_back(variable_l->lb());
        bdu_p.push_back(variable_l->ub());

        if(variable_l->integer())
        {
            if( variable_l->lb() == 0 && variable_l->ub()==1 )
            {
                coltype_p.push_back('B');
            }
            else
            {
                coltype_p.push_back('I');
            }
        }
        else
        {
            coltype_p.push_back('C');
        }
    }
}


//@WARN does not delete the constraints simply remove the coeficients and bounds
void ORTdeactivaterows(operations_research::MPSolver & solver_p, std::vector<int> const & mindex)
{
    const std::vector<operations_research::MPConstraint*> & constraints_l = solver_p.constraints();
    for(int rowInd_l : mindex)
    {
        operations_research::MPConstraint* constraint_l = constraints_l[rowInd_l];
        constraint_l->SetBounds(-solver_p.infinity(), solver_p.infinity());
        constraint_l->Clear();
    }
}

namespace
{
//convert ortools status into an XPRSgetbasis like result value
/**
 * row status :
 *   0 : slack, surplus or artificial is non-basic at lower bound;
 *   1 : slack, surplus or artificial is basic;
 *   2 : slack or surplus is non-basic at upper bound.
 *   3 : slack or surplus is super-basic.
 * column status
 *   0 : variable is non-basic at lower bound, or superbasic at zero if the variable has no lower bound;
 *   1 : variable is basic;
 *   2 : variable is non-basic at upper bound;
 *   3 : variable is super-basic.
*/
    int basisStatusToInt(operations_research::MPSolver::BasisStatus basisStatus_l)
    {
        switch(basisStatus_l)
        {
            case operations_research::MPSolver::FREE :
            {
                return 3;
            }
            case operations_research::MPSolver::AT_LOWER_BOUND :
            {
                return 0;
            }
            case operations_research::MPSolver::AT_UPPER_BOUND :
            {
                return 2;
            }
            case operations_research::MPSolver::FIXED_VALUE :
            {
                return 0; //actually this means 0 and 2 at the same time
            }
            case operations_research::MPSolver::BASIC :
            {
                return 1;
            }
            default:
                std::cerr << "\nbasisStatusToInt: Unknown basis status : " << basisStatus_l << "!";
                return 3;
        }
    }
}

//@WARN I suspect rstatus and cstatus are inversed in ortools xpressinterface implementation using XPRSgetbasis
void ORTgetbasis(operations_research::MPSolver & solver_p, std::vector<int> & rstatus_p, std::vector<int> & cstatus_p)
{
    //row status
    for(auto constraint_l : solver_p.constraints())
    {
        operations_research::MPSolver::BasisStatus rowStatus_l = constraint_l->basis_status();

        if ((rowStatus_l == operations_research::MPSolver::AT_LOWER_BOUND) && (constraint_l->lb() == -solver_p.infinity()))
        {
            double cst_value = 0;
            for(const auto & pairVarCoeff : constraint_l->terms())
            {
                cst_value += pairVarCoeff.second * pairVarCoeff.first->solution_value();
            }
            if(cst_value == constraint_l->ub())
            {
                rowStatus_l = operations_research::MPSolver::AT_UPPER_BOUND;
            }
        }

        rstatus_p.push_back(basisStatusToInt(rowStatus_l));
    }

    //column status
    for(auto variable_l : solver_p.variables())
    {
        operations_research::MPSolver::BasisStatus colStatus_l = variable_l->basis_status();
        cstatus_p.push_back(basisStatusToInt(colStatus_l));
    }
}