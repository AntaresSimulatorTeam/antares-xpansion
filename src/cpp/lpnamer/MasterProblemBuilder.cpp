//
// Created by s90365 on 23/08/2021.
//
#include <solver_utils.h>
#include "MasterProblemBuilder.h"

std::shared_ptr<SolverAbstract> MasterProblemBuilder::build(const std::string& solverName, const std::vector<ActiveLink>& links)
{
	SolverFactory factory;
	auto master_l = factory.create_solver(solverName);

	std::vector<Candidate> candidates;

	for (const auto& link : links)
	{
		const auto& candidateFromLink = link.getCandidates();
		candidates.insert(candidates.end(), candidateFromLink.begin(), candidateFromLink.end());
	}

	AddVariablesPmaxOnEachCandidate(candidates, master_l);

	std::vector<Candidate> candidatesInteger;
	for (int i = 0; i < candidates.size(); i++)
	{
		const auto& candidate = candidates.at(i);
		if (candidate.is_integer()) {
			candidatesInteger.push_back(candidate);
		}
	}

	AddNvarForIntegerCandidate(candidatesInteger, master_l);

	return master_l;
}

void MasterProblemBuilder::AddNvarForIntegerCandidate(std::vector<Candidate>& candidatesInteger, SolverAbstract::Ptr& master_l)
{
	
	int nbNvar = candidatesInteger.size();
	if (nbNvar > 0)
	{
		std::vector<variableIndexes> variableIndexesForIntegerCandidate;
		std::vector<double> zeros(nbNvar, 0.0);
		std::vector<int> mstart(nbNvar, 0);
		std::vector<char> integer_type(nbNvar, 'I');
		std::vector<std::string> colNames(0);
		std::vector<double> max_unit;
		int nIndex = 0;
		for (int i = 0; i < candidatesInteger.size(); i++)
		{
			const auto& candidate = candidatesInteger.at(i);
			max_unit.push_back(candidate.max_unit());
			nIndex++;
		}

		solver_addcols(master_l, zeros, mstart, {}, {}, zeros, max_unit, integer_type, colNames);
	}
}

void MasterProblemBuilder::AddVariablesPmaxOnEachCandidate(std::vector<Candidate>& candidates, SolverAbstract::Ptr& master_l)
{
	int nbCandidates = candidates.size();

	std::vector<int> mstart(nbCandidates, 0);
	std::vector<double> obj_candidate(nbCandidates, 0);
	std::vector<double> lb_candidate(nbCandidates, -1e20);
	std::vector<double> ub_candidate(nbCandidates, 1e20);
	std::vector<char> coltypes_candidate(nbCandidates, 'C');
	std::vector<std::string> candidate_names(nbCandidates);

	for (int i = 0; i < candidates.size(); i++) {
		const auto& candidate = candidates.at(i);
		obj_candidate[i] = candidate.obj();
		lb_candidate[i] = candidate.lb();
		ub_candidate[i] = candidate.ub();
		candidate_names[i] = candidate._data.name;
	}

	solver_addcols(master_l, obj_candidate, mstart, {}, {}, lb_candidate, ub_candidate, coltypes_candidate, candidate_names);
}
