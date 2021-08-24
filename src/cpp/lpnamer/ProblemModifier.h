
#ifndef ANTARESXPANSION_PROBLEMMODIFIER_H
#define ANTARESXPANSION_PROBLEMMODIFIER_H

#include <vector>
#include <memory>
#include <multisolver_interface/SolverAbstract.h>
#include "ActiveLinks.h"
#include <map>
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
                                                  const std::map<linkId,  ColumnsToChange> &p_var_columns);

    unsigned int get_candidate_col_id(const std::string& cand_name) const;

private:

    void changeProblem(const std::vector<ActiveLink> &active_links,
                       const std::map<linkId, ColumnsToChange> &p_var_columns);

    void remove_bounds_for(const ColumnsToChange &columns_to_change);

    void change_upper_bounds_to_pos_inf(const std::vector<int> &col_id);

    void change_lower_bounds_to_neg_inf(const std::vector<int> &col_id);

    void add_new_columns(const std::vector<Candidate> &candidates);

    std::vector<Candidate> candidates_from_all_links(const std::vector<ActiveLink> &active_links) const;

    void add_new_constraints(const std::vector<ActiveLink> &active_links,
                             const std::map<linkId, ColumnsToChange> &p_var_columns);

    std::shared_ptr<SolverAbstract> _math_problem;
    std::map<std::string ,unsigned int> _candidate_col_id;
    unsigned int _n_cols_at_start;
};


#endif //ANTARESXPANSION_PROBLEMMODIFIER_H
