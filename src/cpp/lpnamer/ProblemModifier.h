
#ifndef ANTARESXPANSION_PROBLEMMODIFIER_H
#define ANTARESXPANSION_PROBLEMMODIFIER_H

#include <vector>
#include <memory>
#include <multisolver_interface/SolverAbstract.h>
#include "ActiveLinks.h"
#include <map>
#include <boost/concept_check.hpp>

using colId= unsigned int;
struct ColumnToChange{

    ColumnToChange(colId id, int time_step):id(id),time_step(time_step){};

    bool operator==(const ColumnToChange& other) const{
        bool result = id == other.id;
        result &= time_step == other.time_step;
        return result;
    };

    colId id;
    int time_step;
};

using ColumnsToChange = std::vector<ColumnToChange>;
using linkId=unsigned int;

class ProblemModifier {
public:
    ProblemModifier();

    std::shared_ptr<SolverAbstract> changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const std::vector<ActiveLink> &active_links,
                                                  const std::map<linkId,  ColumnsToChange> &p_ntc_columns,
                                                  const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
                                                  const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns);

    unsigned int get_candidate_col_id(const std::string& cand_name) const;
    bool has_candidate_col_id(const std::string& cand_name) const;

private:

    void changeProblem(const std::vector<ActiveLink> &active_links,
                       const std::map<linkId, ColumnsToChange> &p_ntc_columns,
                       const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
                       const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns);

    void remove_bounds_for(const std::vector<int> &col_ids);

    void change_upper_bounds_to_pos_inf(const std::vector<int> &col_id);

    void change_lower_bounds_to_neg_inf(const std::vector<int> &col_id);

    void add_new_columns(const std::vector<Candidate> &candidates);

    std::vector<Candidate> candidates_from_all_links(const std::vector<ActiveLink> &active_links) const;
    std::set<int> extract_time_steps(const std::map<linkId, ColumnsToChange> &p_columns) const;
    std::vector<Candidate> candidates_with_not_null_profile(const std::vector<ActiveLink> &active_links, const std::set<int>& time_steps) const;

    void add_new_ntc_constraints(const std::vector<ActiveLink> &active_links,
                                 const std::map<linkId, ColumnsToChange> &p_ntc_columns);

    void add_new_direct_cost_constraints(const std::vector<ActiveLink> &active_links,
                             const std::map<linkId, ColumnsToChange> &p_cost_columns);

    void add_new_indirect_cost_constraints(const std::vector<ActiveLink> &active_links,
                                         const std::map<linkId, ColumnsToChange> &p_cost_columns);

    std::shared_ptr<SolverAbstract> _math_problem;
    std::map<std::string ,unsigned int> _candidate_col_id;
    unsigned int _n_cols_at_start;

    void
    add_direct_profile_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                         std::vector<char> &rowtype,
                                         std::vector<double> &rhs, std::vector<int> &rstart, const ActiveLink &link,
                                         const ColumnToChange &column);

    void
    add_indirect_profile_ntc_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                               std::vector<char> &rowtype,
                                               std::vector<double> &rhs, std::vector<int> &rstart, const ActiveLink &link,
                                               const ColumnToChange &column);
    void
    add_indirect_cost_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                        std::vector<char> &rowtype,
                                        std::vector<double> &rhs, std::vector<int> &rstart, const ActiveLink &link,
                                        const ColumnToChange &column);
};


#endif //ANTARESXPANSION_PROBLEMMODIFIER_H
