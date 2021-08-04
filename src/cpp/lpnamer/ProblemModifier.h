
#ifndef ANTARESXPANSION_PROBLEMMODIFIER_H
#define ANTARESXPANSION_PROBLEMMODIFIER_H

#include <vector>
#include <memory>
#include <multisolver_interface/SolverAbstract.h>

struct ColumnToChange{
    int id;
    int time_step;
};

using ColumnsToChange= std::vector<ColumnToChange>;


struct Cand {
    std::string name;
};
using Cands=std::vector<Cand>;

struct ActiveLink{
    unsigned int id;
    std::string name;
    Cands link_candidates;
    ColumnsToChange columns;
};
struct ActiveLinks {
    std::vector<ActiveLink> _links;

    void add_link(ActiveLink link);

    std::vector<ActiveLink> getItems() const;
};



class ProblemModifier {
public:
    ProblemModifier();

    void setProblem(const std::shared_ptr<SolverAbstract> &mathProblem);

    void changeProblem(const ColumnsToChange& columns_to_change);

    void changeProblem(const ActiveLinks& active_links);

private:
    std::shared_ptr<SolverAbstract> math_problem;

    void change_upper_bounds_to_pos_inf(const std::vector<int> &col_id);

    void change_lower_bounds_to_neg_inf(const std::vector<int> &col_id);
};


#endif //ANTARESXPANSION_PROBLEMMODIFIER_H
