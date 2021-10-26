#include "LauncherHelpers.h"


void treatAdditionalConstraints(SolverAbstract::Ptr master_p, 
	const AdditionalConstraints& additionalConstraints_p)
{
	//add requested binary variables
	addBinaryVariables(master_p, additionalConstraints_p.getVariablesToBinarise());

	//add the constraints
	for(auto pairNameAdditionalcstr : additionalConstraints_p )
	{
		addAdditionalConstraint(master_p, pairNameAdditionalcstr.second);
	}
}

void addAdditionalConstraint(SolverAbstract::Ptr master_p, 
	AdditionalConstraint & additionalConstraint_p){
	int newnz = (int)additionalConstraint_p.size();
	int newrows = 1;
	std::vector<char> rtype(newrows);
	std::vector<double> rhs(newrows, additionalConstraint_p.getRHS());
	std::vector<int> mindex(newnz);
	std::vector<double> matval(newnz);
	std::vector<int> matstart(newrows + 1);
	matstart[0] = 0;
	matstart[1] = newnz;

	std::string sign_l = additionalConstraint_p.getSign();
	if ( sign_l == "less_or_equal" ){
		rtype[0] = 'L';
	}
	else if ( sign_l == "greater_or_equal"){
		rtype[0] = 'U';
	}
	else if (sign_l == "equal") {
		rtype[0] = 'E';
	}
	else {
		std::cout << "ERROR un addAdditionalConstraint, unknown row type " 
			<< sign_l << std::endl;
		std::exit(1);
	}

	int i = 0;
	for(auto & pairNameCoeff : additionalConstraint_p)
	{
	    int col_index = master_p->get_col_index(pairNameCoeff.first);
		if( col_index == -1)
		{
			std::cout << "missing variable " << pairNameCoeff.first 
				<< " used in additional constraint file!\n";
			std::exit(1);
		}
		mindex[i] = col_index;
		matval[i] = pairNameCoeff.second;
		i++;
	}

	master_p->add_rows(1, newnz, rtype.data(), rhs.data(), nullptr, matstart.data(), 
		mindex.data(), matval.data());
}


void addBinaryVariables(SolverAbstract::Ptr master_p, const std::map<std::string, 
	std::string>& variablesToBinarise_p){

	for(auto pairOldNewVarnames : variablesToBinarise_p){
	    int col_index = master_p->get_col_index(pairOldNewVarnames.first);

		if (col_index == -1){

			std::cout << "missing variable " << pairOldNewVarnames.first 
				<< " used in additional constraint file!\n";
			std::exit(1);
		}

		master_p->add_cols(1, 0, std::vector<double>(1, 0.0).data(), std::vector<int>(2, 0).data(),
			std::vector<int>(0).data(), std::vector<double>(0).data(),
			std::vector<double>(1, 0).data(), std::vector<double>(1, 1e20).data());

		// Changing column type to binary
		master_p->chg_col_type(std::vector<int>(1, master_p->get_ncols() - 1),
			std::vector<char>(1, 'B'));

		// Changing column name
		master_p->chg_col_name(master_p->get_ncols() - 1, pairOldNewVarnames.second);

		// Add linking constraint
		std::vector<int> matstart(2);
		matstart[0] = 0;
		matstart[1] = 2;
		
		std::vector<int> matind(2);
		matind[0] = col_index;
		matind[1] = master_p->get_ncols() - 1;
		
		std::vector<double> matval(2);
		std::vector<double> oldVarUb(1);
		master_p->get_ub(oldVarUb.data(), col_index, col_index);
		matval[0] = 1;
		matval[1] = -oldVarUb[0];

		master_p->add_rows(1, 2, std::vector<char>(1, 'L').data(), std::vector<double>(1, 0.0).data(),
			nullptr, matstart.data(), matind.data(), matval.data());
		master_p->chg_row_name(master_p->get_nrows() - 1, 
			"link_" + pairOldNewVarnames.first + "_" + pairOldNewVarnames.second);
	}
}
