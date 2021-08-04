
#include "ProblemModifier.h"

#include <utility>

ProblemModifier::ProblemModifier() {}

void ProblemModifier::setProblem(const std::shared_ptr<SolverAbstract> &mathProblem) {
    math_problem = mathProblem;
}

void ProblemModifier::changeProblem(const ColumnsToChange& columns_to_change) {
    std::vector<int> col_ids;
    col_ids.reserve(columns_to_change.size());
    for(auto col:columns_to_change){
        col_ids.push_back(col.id);
    }
    change_upper_bounds_to_pos_inf(col_ids);
    change_lower_bounds_to_neg_inf(col_ids);
}

void ProblemModifier::change_lower_bounds_to_neg_inf(const std::vector<int> &col_ids) {
    std::vector<char> lb_char(col_ids.size(), 'L');
    std::vector<double> neg_inf(col_ids.size(), -1e20);
    math_problem->chg_bounds(col_ids, lb_char, neg_inf);
}

void ProblemModifier::change_upper_bounds_to_pos_inf(const std::vector<int> &col_ids) {
    std::vector<char> ub_char(col_ids.size(), 'U');
    std::vector<double> pos_inf(col_ids.size(), 1e20);
    math_problem->chg_bounds(col_ids, ub_char, pos_inf);
}

void ProblemModifier::changeProblem(const ActiveLinks &active_links) {
    for (const auto& link : active_links.getItems()){
        changeProblem(link.columns);
    }

}

void ActiveLinks::add_link(const ActiveLink link) {
    _links.push_back(link);

}

std::vector<ActiveLink> ActiveLinks::getItems() const {
    return _links;
}
