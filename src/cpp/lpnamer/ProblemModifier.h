
#ifndef ANTARESXPANSION_PROBLEMMODIFIER_H
#define ANTARESXPANSION_PROBLEMMODIFIER_H

#include <vector>
#include <memory>
#include <multisolver_interface/SolverAbstract.h>
#include <map>

struct ColumnToChange{
    int id;
    int time_step;
};

using ColumnsToChange = std::vector<ColumnToChange>;


struct Cand {
    std::string name;
};
using Cands=std::vector<Cand>;

struct ActiveLink_AS{
    unsigned int id;
    Cands candidates;
    ColumnsToChange columns;
};
struct ActiveLinks_AS {
    std::vector<ActiveLink_AS> _links;

    void add_link(const ActiveLink_AS link);

    std::vector<ActiveLink_AS> getItems() const;
};



class ProblemModifier {
public:
    ProblemModifier();

    void remove_bounds_for(const ColumnsToChange &columns_to_change);

    void changeProblem(const ActiveLinks_AS& active_links);
    std::shared_ptr<SolverAbstract> changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const ActiveLinks_AS& active_links);


    std::map<std::string, unsigned int> get_candidate_col_id();

private:
    std::shared_ptr<SolverAbstract> _math_problem;
    std::map<std::string ,unsigned int> _candidate_col_id;
    unsigned int _n_cols_at_start;

    void change_upper_bounds_to_pos_inf(const std::vector<int> &col_id);

    void change_lower_bounds_to_neg_inf(const std::vector<int> &col_id);

    void add_new_columns(const Cands &candidates);

    Cands candidates_from_all_links(const ActiveLinks_AS &active_links) const;
};


#endif //ANTARESXPANSION_PROBLEMMODIFIER_H
