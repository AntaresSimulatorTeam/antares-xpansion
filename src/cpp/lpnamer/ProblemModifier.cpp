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

std::map<std::string, unsigned int> ProblemModifier::get_candidate_col_id() {
    // TODO this method should be removed
    return _candidate_col_id;
}

std::shared_ptr<SolverAbstract>
ProblemModifier::changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const std::vector<ActiveLink> &active_links,
                               const std::map<linkId,  ColumnsToChange> &p_var_columns) {
    _math_problem = std::move(mathProblem);
    _n_cols_at_start = _math_problem->get_ncols();

    changeProblem(active_links, p_var_columns);
    return std::move(_math_problem);
}

void ProblemModifier::changeProblem(const std::vector<ActiveLink> &active_links,
                                    const std::map<linkId, ColumnsToChange> &p_var_columns) {
    for (const auto& link : active_links) {
        remove_bounds_for(p_var_columns.at(link._idLink));
    }
    add_new_columns(candidates_from_all_links(active_links));

    add_new_constraints(active_links, p_var_columns);

}

void ProblemModifier::add_new_constraints(const std::vector<ActiveLink> &active_links,
                                          const std::map<linkId, ColumnsToChange> &p_var_columns) {
    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    for (const auto& link : active_links) {
        for(auto column : p_var_columns.at(link._idLink)){
            rstart.push_back(dmatval.size());
            rhs.push_back(0);
            rowtype.push_back('L');
            colind.push_back(column.id);
            dmatval.push_back(1);
            for (const auto& candidate:link.getCandidates()){
                colind.push_back(_candidate_col_id[candidate._name]);
                dmatval.push_back(-candidate.direct_profile(column.time_step));
            }
            rstart.push_back(dmatval.size());
            rhs.push_back(0);
            rowtype.push_back('G');
            colind.push_back(column.id);
            dmatval.push_back(1);
            for (const auto& candidate:link.getCandidates()){
                colind.push_back(_candidate_col_id[candidate._name]);
                dmatval.push_back(+candidate.direct_profile(column.time_step));
            }

        }
    }
    rstart.push_back(dmatval.size());

    solver_addrows(_math_problem, rowtype, rhs, {}, rstart, colind, dmatval);
}

void ProblemModifier::add_new_columns(const std::vector<Candidate> &candidates) {
    if(!candidates.empty()){
        unsigned int n_candidates = candidates.size();
        std::vector<double> objectives(n_candidates, 0);
        std::vector<double> lb(n_candidates, -1e20);
        std::vector<double> ub(n_candidates, 1e20);
        std::vector<char> coltypes(n_candidates, 'C');
        std::vector<int> mstart(n_candidates, 0);
        std::vector<std::string> candidates_colnames;

        for (const auto& candidate : candidates) {
            candidates_colnames.push_back(candidate._name);
            _candidate_col_id[candidate._name] = _candidate_col_id.size() + _n_cols_at_start;
        }
        solver_addcols(_math_problem, objectives, mstart, {}, {}, lb, ub, coltypes, candidates_colnames);
    }
}

std::vector<Candidate> ProblemModifier::candidates_from_all_links(const std::vector<ActiveLink> &active_links) const {
    std::vector<Candidate> all_candidates;
    for (const auto& link : active_links){
        std::vector<Candidate> candidates = link.getCandidates();
        all_candidates.insert(all_candidates.end(),candidates.begin(), candidates.end());
    }
    return all_candidates;
}



void ActiveLinks_AS::add_link(const ActiveLink_AS link) {
    _links.push_back(link);
}

std::vector<ActiveLink_AS> ActiveLinks_AS::getItems() const {
    return _links;
}
