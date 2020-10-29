#include "LauncherHelpers.h"


/**
 * \brief adds binary variables and additional constraints to an existent solver
 *
 * \param master_p solver to which the constraints and variables will be added
 * \param additionalConstraints_p the additional constraints to add
 */
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

/**
 * \brief adds an additional constraint to an existent solver
 *
 * \param master_p solver to which the constraint will be added
 * \param additionalConstraint_p the additional constraint to add
 */
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

/**
 * \brief creates a binary variable and its corresponding linking constraint
 *
 * \param master_p solver to which the binary variable and the linking constraint will be added
 * \param variablesToBinarise_p map listing the variables to add and their corresponding ones
 *
 * for each entry (BinVar, CorrespondingVar) from the input map,
 *          creates the binary variable BinVar
 *          adds the linking constraint link_BinVar_CorrespondingVar : CorrespondingVar  <= UB(CorrespondingVar) * BinVar
 */
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
