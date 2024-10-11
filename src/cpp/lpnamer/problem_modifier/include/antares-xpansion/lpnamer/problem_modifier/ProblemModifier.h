
#ifndef ANTARESXPANSION_PROBLEMMODIFIER_H
#define ANTARESXPANSION_PROBLEMMODIFIER_H

#include "antares-xpansion/multisolver_interface/SolverAbstract.h"

#include <map>
#include <memory>
#include <vector>

#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/helper/ColumnToChange.h"
#include "LogUtils.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"

class ProblemModifier {
 public:
  ProblemModifier(
      ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger)
      : logger_(logger) {}

  void changeProblem(
      Problem *problem, const std::vector<ActiveLink> &active_links,
      const std::map<linkId, ColumnsToChange> &p_ntc_columns,
      const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
      const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns);

  unsigned int get_candidate_col_id(const std::string &cand_name) const;
  bool has_candidate_col_id(const std::string &cand_name) const;
  class CandidateWasNotAddedInProblem
      : public LogUtils::XpansionError<std::runtime_error> {
    using LogUtils::XpansionError<std::runtime_error>::XpansionError;
  };

 private:
  void changeProblem(
      const std::vector<ActiveLink> &active_links,
      const std::map<linkId, ColumnsToChange> &p_ntc_columns,
      const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
      const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns);

  void remove_bounds_for(const std::vector<int> &col_ids) const;

  void change_upper_bounds_to_pos_inf(const std::vector<int> &col_id) const;

  void change_lower_bounds_to_neg_inf(const std::vector<int> &col_id) const;

  void add_new_columns(const std::vector<Candidate> &candidates);

  void add_new_ntc_constraints(
      const std::vector<ActiveLink> &active_links,
      const std::map<linkId, ColumnsToChange> &p_ntc_columns);

  void add_new_direct_cost_constraints(
      const std::vector<ActiveLink> &active_links,
      const std::map<linkId, ColumnsToChange> &p_cost_columns);

  void add_new_indirect_cost_constraints(
      const std::vector<ActiveLink> &active_links,
      const std::map<linkId, ColumnsToChange> &p_cost_columns);

  Problem *_math_problem;
  std::map<std::string, unsigned int> _candidate_col_id;
  unsigned int _n_cols_at_start = 0;

  void add_direct_profile_column_constraint(std::vector<double> &dmatval,
                                            std::vector<int> &colind,
                                            std::vector<char> &rowtype,
                                            std::vector<double> &rhs,
                                            std::vector<int> &rstart,
                                            const ActiveLink &link,
                                            const ColumnToChange &column);

  void add_indirect_profile_ntc_column_constraint(std::vector<double> &dmatval,
                                                  std::vector<int> &colind,
                                                  std::vector<char> &rowtype,
                                                  std::vector<double> &rhs,
                                                  std::vector<int> &rstart,
                                                  const ActiveLink &link,
                                                  const ColumnToChange &column);
  void add_indirect_cost_column_constraint(std::vector<double> &dmatval,
                                           std::vector<int> &colind,
                                           std::vector<char> &rowtype,
                                           std::vector<double> &rhs,
                                           std::vector<int> &rstart,
                                           const ActiveLink &link,
                                           const ColumnToChange &column);
  bool candidateContributionDirectIsNotNull(const ColumnToChange &column,
                                            unsigned int chronicle_to_use,
                                            const Candidate &candidate) const;
  unsigned int chronicleToUse(const ActiveLink &link) const;
  std::vector<Candidate> candidates_with_not_null_profile(
      const std::vector<ActiveLink> &active_links,
      const std::set<int> &time_steps) const;

  bool candidateContributionIndirectIsNotNull(const ColumnToChange &column,
                                              unsigned int chronicle_to_use,
                                              const Candidate &candidate) const;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger_;
};

#endif  // ANTARESXPANSION_PROBLEMMODIFIER_H
