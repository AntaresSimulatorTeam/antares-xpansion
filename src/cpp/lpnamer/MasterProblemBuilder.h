//
// Created by s90365 on 23/08/2021.
//

#include <multisolver_interface/SolverAbstract.h>
#include "ActiveLinks.h"

#ifndef ANTARESXPANSION_MASTERPROBLEMBUILDER_H
#define ANTARESXPANSION_MASTERPROBLEMBUILDER_H

struct variableIndexes
{
	int pMaxIndex;
	int nVarIndex;
};

class MasterProblemBuilder {
public:
	std::shared_ptr<SolverAbstract> build(const std::string& solverName, const std::vector<ActiveLink>& links);
	void AddNvarForIntegerCandidate(std::vector<Candidate>& candidatesInteger, SolverAbstract::Ptr& master_l);
private :
	void AddVariablesPmaxOnEachCandidate(std::vector<Candidate>& candidates, SolverAbstract::Ptr& master_l);
};

#endif //ANTARESXPANSION_MASTERPROBLEMBUILDER_H
