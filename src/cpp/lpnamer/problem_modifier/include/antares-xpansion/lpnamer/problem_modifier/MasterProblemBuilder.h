//
// Created by s90365 on 23/08/2021.
//

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

#include <filesystem>
#include <unordered_map>

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"

#ifndef ANTARESXPANSION_MASTERPROBLEMBUILDER_H
#define ANTARESXPANSION_MASTERPROBLEMBUILDER_H

const std::string NB_UNITS_VAR_PREFIX("nb_units_");

class MasterProblemBuilder {
 public:
  explicit MasterProblemBuilder(std::string master_formulation);
  std::shared_ptr<SolverAbstract> build(
      const std::string& solverName, const std::vector<Candidate>& candidates,
      SolverLogManager& solver_log_manager);

 private:
  void addNvarOnEachIntegerCandidate(
      const std::vector<Candidate>& candidatesInteger,
      SolverAbstract::Ptr& master_l) const;
  void addVariablesPmaxOnEachCandidate(const std::vector<Candidate>& candidates,
                                       SolverAbstract::Ptr& master_l);
  void addPmaxConstraint(const std::vector<Candidate>& candidatesInteger,
                         SolverAbstract& master_l);
  int getPmaxVarColumnNumberFor(const Candidate& candidate);

  std::unordered_map<std::string, int> _indexOfPmaxVar;
  std::unordered_map<std::string, int> _indexOfNvar;
  std::string _master_formulation;
};

#endif  // ANTARESXPANSION_MASTERPROBLEMBUILDER_H
