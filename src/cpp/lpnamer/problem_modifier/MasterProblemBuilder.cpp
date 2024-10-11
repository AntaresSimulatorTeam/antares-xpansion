//
// Created by s90365 on 23/08/2021.
//
#include "MasterProblemBuilder.h"

#include <antares-xpansion/helpers/solver_utils.h>

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "LogUtils.h"

MasterProblemBuilder::MasterProblemBuilder(std::string master_formulation)
    : _master_formulation(std::move(master_formulation)) {}

std::shared_ptr<SolverAbstract> MasterProblemBuilder::build(
    const std::string& solverName, const std::vector<Candidate>& candidates,
    SolverLogManager& solver_log_manager) {
  _indexOfNvar.clear();
  _indexOfPmaxVar.clear();

  SolverFactory factory;
  auto master_l = factory.create_solver(solverName, solver_log_manager);

  addVariablesPmaxOnEachCandidate(candidates, master_l);

  std::vector<Candidate> candidatesInteger;

  std::copy_if(candidates.begin(), candidates.end(),
               back_inserter(candidatesInteger),
               [](const Candidate& cand) { return cand.is_integer(); });

  if (_master_formulation == "integer") {
    addNvarOnEachIntegerCandidate(candidatesInteger, master_l);

    addPmaxConstraint(candidatesInteger, *master_l);
  }

  return master_l;
}

void MasterProblemBuilder::addPmaxConstraint(
    const std::vector<Candidate>& candidatesInteger, SolverAbstract& master_l) {
  auto n_integer = static_cast<unsigned>(candidatesInteger.size());
  if (n_integer > 0) {
    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;
    dmatval.reserve(2 * n_integer);
    colind.reserve(2 * n_integer);
    rowtype.reserve(n_integer);
    rhs.reserve(n_integer);
    rstart.reserve(n_integer + 1);

    int positionInIntegerCandidadeList(0);
    auto nbColPmaxVar = (int)_indexOfPmaxVar.size();

    for (const auto& candidate : candidatesInteger) {
      int pmaxVarColumNumber = getPmaxVarColumnNumberFor(candidate);
      int nVarColumNumber = nbColPmaxVar + positionInIntegerCandidadeList;

      // pMax  - n unit_size = 0
      rstart.push_back(static_cast<int>(dmatval.size()));
      rhs.push_back(0);
      rowtype.push_back('E');

      colind.push_back(pmaxVarColumNumber);
      dmatval.push_back(1);

      colind.push_back(nVarColumNumber);
      dmatval.push_back(-candidate.unit_size());

      positionInIntegerCandidadeList++;
    }
    rstart.push_back((int)dmatval.size());

    solver_addrows(master_l, rowtype, rhs, {}, rstart, colind, dmatval);
  }
}

int MasterProblemBuilder::getPmaxVarColumnNumberFor(
    const Candidate& candidate) {
  const auto& it = _indexOfPmaxVar.find(candidate.get_name());
  if (it == _indexOfPmaxVar.end()) {
    auto message = LOGLOCATION + "There is no Pvar for the candidate " +
                   candidate.get_name();
    throw std::runtime_error(message);
  }
  return it->second;
}

void MasterProblemBuilder::addNvarOnEachIntegerCandidate(
    const std::vector<Candidate>& candidatesInteger,
    SolverAbstract::Ptr& master_l) const {
  auto nbNvar = (int)candidatesInteger.size();
  if (nbNvar > 0) {
    std::vector<double> zeros(nbNvar, 0.0);
    std::vector<int> mstart(nbNvar, 0);
    std::vector<char> integer_type(nbNvar, 'I');
    std::vector<std::string> colNames;
    std::vector<double> max_unit;

    for (int i = 0; i < candidatesInteger.size(); i++) {
      const auto& candidate = candidatesInteger.at(i);
      max_unit.push_back(candidate.max_unit());
      colNames.push_back(NB_UNITS_VAR_PREFIX + candidate.get_name());
    }

    solver_addcols(*master_l, zeros, mstart, {}, {}, zeros, max_unit,
                   integer_type, colNames);
  }
}

void MasterProblemBuilder::addVariablesPmaxOnEachCandidate(
    const std::vector<Candidate>& candidates, SolverAbstract::Ptr& master_l) {
  auto nbCandidates = (int)candidates.size();

  if (nbCandidates > 0) {
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
      candidate_names[i] = candidate.get_name();
      _indexOfPmaxVar[candidate.get_name()] = i;
    }

    solver_addcols(*master_l, obj_candidate, mstart, {}, {}, lb_candidate,
                   ub_candidate, coltypes_candidate, candidate_names);
  }
}
