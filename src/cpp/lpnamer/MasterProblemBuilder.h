//
// Created by s90365 on 23/08/2021.
//

#include <multisolver_interface/SolverAbstract.h>
#include <unordered_map>
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
	std::shared_ptr<SolverAbstract> build(const std::string& solverName, const std::vector<Candidate>& candidates);

	
	
private :
	void addNvarForIntegerCandidate(const std::vector<Candidate>& candidatesInteger, SolverAbstract::Ptr& master_l);
	void addVariablesPmaxOnEachCandidate(const std::vector<Candidate>& candidates, SolverAbstract::Ptr& master_l);
	void addPmaxConstraint(const std::vector<Candidate>& candidatesInteger, const std::vector<Candidate>& candidates, SolverAbstract::Ptr& master_l);
	int getPmaxVarColumnNumberFor(const Candidate& candidate);

	std::unordered_map<std::string, int> _indexOfPvar;
	std::unordered_map<std::string, int> _indexOfNvar;

};

#endif //ANTARESXPANSION_MASTERPROBLEMBUILDER_H
