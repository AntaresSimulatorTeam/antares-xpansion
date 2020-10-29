#include <fstream>

#include "ortools_utils.h"
#include "ortools/lp_data/mps_reader.h"
#include "ortools/lp_data/proto_utils.h"
#include "ortools/linear_solver/linear_solver.pb.h"
#include "ortools/linear_solver/model_exporter.h"


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
                std::cerr << "\nbasisStatusToInt: Unknown basis status : " << basisStatus_l << "!\n";
                return 3;
        }
    }
}

operations_research::MPSolverResponseStatus ORTreadmps(operations_research::MPSolver & solver_p, std::string const & filename_p)
{
    solver_p.Clear();

    std::ifstream mpsfile(filename_p.c_str());
    if(mpsfile.good())
    {
        operations_research::MPModelProto model_proto_l;
#if defined(ORTOOLS_PRE_V71)
        operations_research::glop::LinearProgram linearProgram_l;
        operations_research::glop::MPSReader().LoadFileWithMode(filename_p, true, &linearProgram_l);
        operations_research::glop::LinearProgramToMPModelProto(linearProgram_l, &model_proto_l);
#else
        operations_research::glop::MPSReader().ParseFile(filename_p, &model_proto_l);
#endif
        std::string errorMessage_l;
        const operations_research::MPSolverResponseStatus status = solver_p.LoadModelFromProtoWithUniqueNamesOrDie(model_proto_l, &errorMessage_l);
        if(errorMessage_l.length())
        {
            std::cerr << "readMPS::error message: " << errorMessage_l << std::endl;
        }
        else
        {
            if (solver_p.NumVariables() == 0)
                std::cout << "readMPS:: no variable in mps " << filename_p << std::endl;
            if (solver_p.NumConstraints() == 0)
                std::cout << "readMPS:: no constraint in mps " << filename_p << std::endl;
        }
        return status;
    }

    std::cerr << "MPS file " << filename_p << " was not found!\n";
    return operations_research::MPSOLVER_MODEL_INVALID;
}

bool ORTwritemps(operations_research::MPSolver const & solver_p, std::string const & filename_p)
{
    std::string modelMps_l;
    solver_p.ExportModelAsMpsFormat(false, false, &modelMps_l);

    std::ofstream mpsOut(filename_p);
    mpsOut <<  modelMps_l;
    mpsOut.close();
	return true;
}

bool ORTwritelp(operations_research::MPSolver const & solver_p, std::string const & filename_p)
{
    std::string modelLP_l;
    solver_p.ExportModelAsLpFormat(false, &modelLP_l);

    std::ofstream lpOut(filename_p);
    lpOut <<  modelLP_l;
    lpOut.close();
	return true;
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


void ORTgetrows(operations_research::MPSolver const & solver_p,
                std::vector<int> & mstart_p,
                std::vector<int> & mclind_p,
                std::vector<double> & dmatval_p,
                int first_p, int last_p)
{
    mstart_p.clear();
    mclind_p.clear();
    dmatval_p.clear();

    int ind(0);
    for(auto itConstraint_l(solver_p.constraints().cbegin()+first_p), itConstraintEnd_l(solver_p.constraints().cbegin()+last_p+1) ;
         itConstraint_l!=itConstraintEnd_l ; ++itConstraint_l)
    {
        operations_research::MPConstraint* constraint_l(*itConstraint_l);
        mstart_p.push_back(ind);

        std::for_each(constraint_l->terms().begin(), constraint_l->terms().end(),
                        [&ind, &mclind_p, &dmatval_p]
                        (std::pair<const operations_research::MPVariable*, double> const & termVarVal_p){
                            mclind_p.push_back(termVarVal_p.first->index());
                            dmatval_p.push_back(termVarVal_p.second);
                            ++ind;
                        });

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

	auto const & mapVarCoeff = solver_p.Objective().terms();
    std::transform(solver_p.variables().cbegin()+first_p, solver_p.variables().cbegin()+last_p+1,
                    std::back_inserter(obj_p),
                    [&mapVarCoeff](operations_research::MPVariable * const variable_p) -> double{
                        auto it_l = mapVarCoeff.find(variable_p);
                        if ( it_l != mapVarCoeff.end() )
                        {
                            return it_l->second;
                        }
                        else
                        {
                            return 0;
                        }
                    });
}

void ORTaddcols(operations_research::MPSolver & solver_p,
                std::vector<double> const & objx_p,
                std::vector<int> const & mstart_p,
                std::vector<int> const & mrwind_p,
                std::vector<double> const & dmatval_p,
                std::vector<double> const & bdl_p, std::vector<double> const & bdu_p,
                std::vector<char> const & colTypes_p,
                std::vector<std::string> const & colNames_p)
{
	assert(objx_p.size() != 0);
    assert((objx_p.size() == mstart_p.size()) || (mstart_p.size() == 0));
    assert(mrwind_p.size() == dmatval_p.size());

	operations_research::MPObjective* objective_l = solver_p.MutableObjective();
	const std::vector<operations_research::MPConstraint*> & constraints_l = solver_p.constraints();

	for(int col_l(0); col_l < objx_p.size(); ++col_l)
	{
		const std::string& name_l = (colNames_p.size() == objx_p.size()) ? colNames_p[col_l] : "";
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
				std::cerr << "type of the variable " << col_l << " is not handled : -" << colTypes_p[col_l] << "-!\n" ;
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

void ORTaddrows(operations_research::MPSolver & solver_p,
                std::vector<char> const &  qrtype_p,
                std::vector<double>  const & rhs_p,
                std::vector<double>  const & range_p,
                std::vector<int> const & mstart_p,
                std::vector<int> const & mclind_p,
                std::vector<double> const & dmatval_p)
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
		const std::string& name_l = "addedRow_" + std::to_string(solver_p.NumConstraints());

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
                    std::cerr << "ORTaddrows: negative range values are not handled!\n";
                }
                break;
			}
			case 'N':
			{
				std::cout << "ORTaddrows: ignoring non-binding row " << row_l << ".\n";
                continue;//ignore non-binding rows
                break;
			}
			default:
			{
				std::cerr << "type of the row " << row_l << " is not handled : " << qrtype_p[row_l] << "!\n" ;
			}
		}

		operations_research::MPConstraint* const mpConstraint_l = solver_p.MakeRowConstraint(lb_l, ub_l, name_l);
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

void ORTgetlpsolution(operations_research::MPSolver const & solver_p, std::vector<double> & x_p)
{
    x_p.clear();

	const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
	std::transform(variables_l.begin(), variables_l.end(),
                   std::back_inserter(x_p),
                   [](operations_research::MPVariable * const var_l) -> double{
                       return var_l->solution_value();
                   });
}

void ORTgetlpdual(operations_research::MPSolver const & solver_p, std::vector<double> & dual_p)
{
    dual_p.clear();

	const std::vector<operations_research::MPConstraint*> & constraints_l = solver_p.constraints();
    std::transform(constraints_l.begin(), constraints_l.end(),
                   std::back_inserter(dual_p),
                   [](operations_research::MPConstraint * const cstr_l) -> double{
                       return cstr_l->dual_value();
                   });
}

void ORTgetlpreducedcost(operations_research::MPSolver const & solver_p, std::vector<double> & dj_p)
{
    dj_p.clear();

    const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
	std::transform(variables_l.begin(), variables_l.end(),
                   std::back_inserter(dj_p),
                   [](operations_research::MPVariable * const var_l) -> double{
                        return var_l->reduced_cost();
                   });
}

void ORTgetrowtype(operations_research::MPSolver const & solver_p, std::vector<char> & qrtype_p, int first_p, int last_p)
{
    qrtype_p.clear();

    std::transform(solver_p.constraints().cbegin()+first_p, solver_p.constraints().cbegin()+last_p+1,
                   std::back_inserter(qrtype_p),
                   [&solver_p](operations_research::MPConstraint * const cstr_l) -> char{
                        if( (cstr_l->lb() == -solver_p.infinity()) && (cstr_l->ub() == solver_p.infinity()) )
                        {
                            return 'N';
                        }
                        if( cstr_l->lb() == -solver_p.infinity() )
                        {
                            return 'L';
                        }
                        else if ( cstr_l->ub() == solver_p.infinity())
                        {
                            return 'G';
                        }
                        else if ( cstr_l->lb() == cstr_l->ub() )
                        {
                            return 'E';
                        }
                        else
                        {
                            return 'R';
                        }
                   });
}

void ORTgetrhs(operations_research::MPSolver const & solver_p, std::vector<double> & rhs_p, int first_p, int last_p)
{
    rhs_p.clear();

    std::transform(solver_p.constraints().cbegin()+first_p, solver_p.constraints().cbegin()+last_p+1,
                   std::back_inserter(rhs_p),
                   [&solver_p](operations_research::MPConstraint * const cstr_l) -> double{
                        if( cstr_l->lb() == -solver_p.infinity() )
                        {
                            return cstr_l->ub();
                        }
                        else if ( cstr_l->ub() == solver_p.infinity())
                        {
                            return cstr_l->lb();
                        }
                        else if ( cstr_l->lb() == cstr_l->ub() )
                        {
                            return cstr_l->lb();
                        }
                        else
                        {
                            //TODO : we assume that the RHS for ranges is the ub : verify consistency with xpress results
                            return cstr_l->ub();
                        }
                   });
}

void ORTgetrhsrange(operations_research::MPSolver const & solver_p, std::vector<double> &  range_p, int first_p, int last_p)
{
    range_p.clear();

    std::transform(solver_p.constraints().cbegin()+first_p, solver_p.constraints().cbegin()+last_p+1,
                   std::back_inserter(range_p),
                   [&solver_p](operations_research::MPConstraint * const cstr_l) -> double{
                        if( (cstr_l->lb() == -solver_p.infinity()) ||  ( cstr_l->ub() == solver_p.infinity()) )
                        {
                            return solver_p.infinity();
                        }
                        else
                        {
                            return (cstr_l->ub() - cstr_l->lb());
                        }
                   });
}

void ORTgetcolinfo(operations_research::MPSolver const & solver_p,
                    std::vector<char> & coltype_p,
                    std::vector<double> & bdl_p, std::vector<double> & bdu_p,
                    int first_p, int last_p)
{
    bdl_p.clear();
    bdu_p.clear();
    coltype_p.clear();

    std::for_each(solver_p.variables().cbegin()+first_p, solver_p.variables().cbegin()+last_p+1,
                    [&bdl_p, &bdu_p, &coltype_p](operations_research::MPVariable* const variable_l){
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
                    });
}


//@WARN does not delete the constraints simply removes the coeficients and bounds
void ORTdeactivaterows(operations_research::MPSolver & solver_p, std::vector<int> const & mindex)
{
    const std::vector<operations_research::MPConstraint*> & constraints_l = solver_p.constraints();
    std::for_each(mindex.begin(), mindex.end(),
                    [&constraints_l, &solver_p](int rowInd_l){
                        operations_research::MPConstraint* cstr_l = constraints_l[rowInd_l];
                        cstr_l->SetBounds(-solver_p.infinity(), solver_p.infinity());
                        cstr_l->Clear();
                    });
}


//@WARN rstatus and cstatus are inversed in ortools xpressinterface implementation using XPRSgetbasis,
//check if ortools fixed this issue if having bad results with xpress
void ORTgetbasis(operations_research::MPSolver & solver_p, std::vector<int> & rstatus_p, std::vector<int> & cstatus_p)
{
    //row status
    std::transform(solver_p.constraints().begin(), solver_p.constraints().end(),
                    std::back_inserter(rstatus_p),
                    [&solver_p](const operations_research::MPConstraint * const cstr_l) -> int{
                            operations_research::MPSolver::BasisStatus rowStatus_l = cstr_l->basis_status();

                            if ((rowStatus_l == operations_research::MPSolver::AT_LOWER_BOUND) && (cstr_l->lb() == -solver_p.infinity()))
                            {
                                double cst_value = 0;
                                for(const auto & pairVarCoeff : cstr_l->terms())
                                {
                                    cst_value += pairVarCoeff.second * pairVarCoeff.first->solution_value();
                                }
                                if(cst_value == cstr_l->ub())
                                {
                                    rowStatus_l = operations_research::MPSolver::AT_UPPER_BOUND;
                                }
                            }

                            return basisStatusToInt(rowStatus_l);
                    });

    //column status
    std::transform(solver_p.variables().begin(), solver_p.variables().end(),
                    std::back_inserter(cstatus_p),
                    [](const operations_research::MPVariable * const variable_l) -> int{
                        return basisStatusToInt(variable_l->basis_status());
                    });
}

void ORTchgbounds(operations_research::MPSolver & solver_p,
                  std::vector<int> const & mindex_p,
                  std::vector<char> const & qbtype_p,
                  std::vector<double> const & bnd_p)
{
    assert(mindex_p.size() == qbtype_p.size());
    assert(mindex_p.size() == bnd_p.size());

    const std::vector<operations_research::MPVariable*> & variables_l = solver_p.variables();
    int cnt_l(0);
    for(int index_l : mindex_p)
    {
        switch(qbtype_p[cnt_l])
        {
            case 'U' :
            {
                variables_l[index_l]->SetUB(bnd_p[cnt_l]);
                break;
            }
            case 'L' :
            {
                variables_l[index_l]->SetLB(bnd_p[cnt_l]);
                break;
            }
            case 'B' :
            {
                variables_l[index_l]->SetBounds(bnd_p[cnt_l], bnd_p[cnt_l]);
                break;
            }
            default:
                std::cerr << "\nORTchgbounds: Unknown bound type : " << qbtype_p[cnt_l] << "!\n";
        }
        ++cnt_l;
    }
}


void ORTcopyandrenamevars(operations_research::MPSolver & outSolver_p,
                          operations_research::MPSolver const & inSolver_p,
                          std::vector<std::string> const & names_p)
{
    if (outSolver_p.ProblemType() != inSolver_p.ProblemType())
    {
        std::cout << "\nWarn: ORTcopyandrenamevars is copying solvers with different types!\n";
    }

    outSolver_p.Clear();

    //copy and rename columns
    std::vector<double> obj_l;
	ORTgetobj(inSolver_p, obj_l, 0, inSolver_p.NumVariables() - 1);
    std::vector<double> lb_l;
	std::vector<double> ub_l;
	std::vector<char> coltype_l;
	ORTgetcolinfo(inSolver_p, coltype_l, lb_l, ub_l, 0, inSolver_p.NumVariables() - 1);
	ORTaddcols(outSolver_p, obj_l, {}, {}, {}, lb_l, ub_l, coltype_l, names_p);

    const std::vector<operations_research::MPVariable*> & outVariables_l = outSolver_p.variables();
    assert(inSolver_p.NumVariables() == outVariables_l.size());

    //copy constraints
    for(auto inConstraint_l : inSolver_p.constraints())
    {
        operations_research::MPConstraint* outConstraint_l = outSolver_p.MakeRowConstraint(inConstraint_l->lb(), inConstraint_l->ub(), inConstraint_l->name());
        for(auto pairVarCoeff_l : inConstraint_l->terms())
        {
            outConstraint_l->SetCoefficient(outVariables_l[pairVarCoeff_l.first->index()], pairVarCoeff_l.second);
        }
    }
}
