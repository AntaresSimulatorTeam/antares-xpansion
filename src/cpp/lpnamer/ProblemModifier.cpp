#include "solver_utils.h"
#include "ProblemModifier.h"

ProblemModifier::ProblemModifier() = default;


void ProblemModifier::remove_bounds_for(const ColumnsToChange &columns_to_change) {
    std::vector<int> col_ids;
    col_ids.reserve(columns_to_change.size());
    for(auto col:columns_to_change){
        col_ids.push_back(col.id);
    }
    change_upper_bounds_to_pos_inf(col_ids);
    change_lower_bounds_to_neg_inf(col_ids);
}

void ProblemModifier::changeProblem(const ActiveLinks &active_links) {
    for (const auto& link : active_links.getItems()) {
        remove_bounds_for(link.columns);
    }
    add_new_columns(candidates_from_all_links(active_links));

}

Cands ProblemModifier::candidates_from_all_links(const ActiveLinks &active_links) const {
    Cands all_candidates;
    for (const auto& link : active_links.getItems()){
        auto candidates = link.candidates;
        all_candidates.insert(all_candidates.end(),candidates.begin(), candidates.end());
    }
    return all_candidates;
}

void ProblemModifier::add_new_columns(const Cands &candidates) {
    if(!candidates.empty()){
        unsigned int n_candidates = candidates.size();
        std::vector<double> objectives(n_candidates, 0);
        std::vector<double> lb(n_candidates, -1e20);
        std::vector<double> ub(n_candidates, 1e20);
        std::vector<char> coltypes(n_candidates, 'C');
        std::vector<int> mstart(n_candidates, 0);
        std::vector<std::string> candidates_colnames;

        for (const auto& candidate : candidates) {
            candidates_colnames.push_back(candidate.name);
            _candidate_col_id[candidate.name] = _candidate_col_id.size() + _n_cols_at_start;
        }
        solver_addcols(_math_problem, objectives, mstart, {}, {}, lb, ub, coltypes, candidates_colnames);
    }
}

std::shared_ptr<SolverAbstract>
ProblemModifier::changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const ActiveLinks& active_links) {
    _math_problem = std::move(mathProblem);
    _n_cols_at_start = _math_problem->get_ncols();

    changeProblem(active_links);
    return std::move(_math_problem);
}

void ProblemModifier::change_lower_bounds_to_neg_inf(const std::vector<int> &col_ids) {
    std::vector<char> lb_char(col_ids.size(), 'L');
    std::vector<double> neg_inf(col_ids.size(), -1e20);
    _math_problem->chg_bounds(col_ids, lb_char, neg_inf);
}

void ProblemModifier::change_upper_bounds_to_pos_inf(const std::vector<int> &col_ids) {
    std::vector<char> ub_char(col_ids.size(), 'U');
    std::vector<double> pos_inf(col_ids.size(), 1e20);
    _math_problem->chg_bounds(col_ids, ub_char, pos_inf);
}





void ActiveLinks::add_link(const ActiveLink link) {
    _links.push_back(link);

}

std::vector<ActiveLink> ActiveLinks::getItems() const {
    return _links;
}
