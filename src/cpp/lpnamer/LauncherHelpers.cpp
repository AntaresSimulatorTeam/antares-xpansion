#include "LauncherHelpers.h"


void treatAdditionalConstraints(operations_research::MPSolver & master_p, AdditionalConstraints additionalConstraints_p)
{
	//add requested binary variables
	addBinaryVariables(master_p, additionalConstraints_p.getVariablesToBinarise());

	//add the constraints
	for(auto pairNameAdditionalcstr : additionalConstraints_p )
	{
		addAdditionalConstraint(master_p, pairNameAdditionalcstr.second);
	}
}

void addAdditionalConstraint(operations_research::MPSolver & master_p, AdditionalConstraint & additionalConstraint_p)
{
	operations_research::MPConstraint* newCstr_l = master_p.MakeRowConstraint(additionalConstraint_p.getName());

	std::string sign_l = additionalConstraint_p.getSign();
	if ( (sign_l == "less_or_equal") || (sign_l == "equal") )
	{
		newCstr_l->SetUB(additionalConstraint_p.getRHS());
	}
	if ( (sign_l == "greater_or_equal") || (sign_l == "equal") )
	{
		newCstr_l->SetLB(additionalConstraint_p.getRHS());
	}

	for(auto & pairNameCoeff : additionalConstraint_p)
	{
		operations_research::MPVariable * var_l = master_p.LookupVariableOrNull(pairNameCoeff.first);
		if( nullptr == var_l )
		{
			std::cout << "missing variable " << pairNameCoeff.first << " used in additional constraint file!\n";
		}
		newCstr_l->SetCoefficient(var_l, pairNameCoeff.second);
	}
}


void addBinaryVariables(operations_research::MPSolver & master_p, std::map<std::string, std::string> const & variablesToBinarise_p)
{
	for(auto pairOldNewVarnames : variablesToBinarise_p)
	{
		operations_research::MPVariable * oldVar_l = master_p.LookupVariableOrNull(pairOldNewVarnames.first);
		if ( nullptr == oldVar_l )
		{
			std::cout << "missing variable " << pairOldNewVarnames.first << " used in additional constraint file!\n";
			std::exit(1);
		}

		operations_research::MPVariable * binaryVar_l = master_p.MakeBoolVar(pairOldNewVarnames.second);
		operations_research::MPConstraint* linkCstr_l = master_p.MakeRowConstraint(-operations_research::MPSolver::infinity(),0,
																	"link_"+pairOldNewVarnames.first+"_"+pairOldNewVarnames.second);
		linkCstr_l->SetCoefficient(oldVar_l, 1);
		linkCstr_l->SetCoefficient(binaryVar_l, -oldVar_l->ub());
	}
}
