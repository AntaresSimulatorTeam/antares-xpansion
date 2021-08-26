#include <algorithm>
#include "solver_utils.h"
#include "ProblemModifier.h"

ProblemModifier::ProblemModifier() = default;

std::vector<int> extract_col_ids(const ColumnsToChange &columns_to_change) {
    std::vector<int> col_ids;
    col_ids.reserve(columns_to_change.size());
    std::transform(columns_to_change.begin(),columns_to_change.end(), std::back_inserter(col_ids),[](const ColumnToChange& col){return col.id;});
    return col_ids;
}

void ProblemModifier::remove_bounds_for(const std::vector<int> &col_ids) {
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

unsigned int ProblemModifier::get_candidate_col_id(const std::string& cand_name) const {
    if (_candidate_col_id.find(cand_name) == _candidate_col_id.end()){
        throw std::runtime_error("Candidate '" + cand_name + "' not added in problem");
    }
    return _candidate_col_id.at(cand_name);
}

std::shared_ptr<SolverAbstract>
ProblemModifier::changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const std::vector<ActiveLink> &active_links,
                               const std::map<linkId,  ColumnsToChange> &p_ntc_columns,
                               const std::map<linkId, ColumnsToChange> &p_cost_columns) {
    _math_problem = std::move(mathProblem);
    _n_cols_at_start = _math_problem->get_ncols();

    changeProblem(active_links, p_ntc_columns, p_cost_columns);
    return std::move(_math_problem);
}

void ProblemModifier::changeProblem(const std::vector<ActiveLink> &active_links,
                                    const std::map<linkId, ColumnsToChange> &p_ntc_columns,
                                    const std::map<linkId, ColumnsToChange> &p_cost_columns) {
    for (const auto& link : active_links) {
        remove_bounds_for(extract_col_ids(p_ntc_columns.at(link._idLink)));

        if (p_cost_columns.find(link._idLink) != p_cost_columns.end()) {
            change_upper_bounds_to_pos_inf(extract_col_ids(p_cost_columns.at(link._idLink)));
        }
    }
    add_new_columns(candidates_from_all_links(active_links));

    add_new_ntc_constraints(active_links, p_ntc_columns);
    add_new_cost_constraints(active_links, p_cost_columns);

}

void ProblemModifier::add_new_ntc_constraints(const std::vector<ActiveLink> &active_links,
                                              const std::map<linkId, ColumnsToChange> &p_ntc_columns) {
    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    for (const auto& link : active_links) {
        for(auto column : p_ntc_columns.at(link._idLink)){
            rstart.push_back(dmatval.size());

            double already_installed_capacity( link._already_installed_capacity);
            double direct_already_installed_profile_at_timestep = link.already_installed_direct_profile(column.time_step);

            rhs.push_back(already_installed_capacity * direct_already_installed_profile_at_timestep);
            rowtype.push_back('L');
            colind.push_back(column.id);
            dmatval.push_back(1);
            for (const auto& candidate:link.getCandidates()){
                colind.push_back(_candidate_col_id[candidate._name]);
                dmatval.push_back(-candidate.direct_profile(column.time_step));
            }
            double indirect_already_installed_profile_at_timestep = link.already_installed_indirect_profile(column.time_step);

            rstart.push_back(dmatval.size());
            rhs.push_back(-already_installed_capacity*indirect_already_installed_profile_at_timestep);
            rowtype.push_back('G');
            colind.push_back(column.id);
            dmatval.push_back(1);
            for (const auto& candidate:link.getCandidates()){
                colind.push_back(_candidate_col_id[candidate._name]);
                dmatval.push_back(+candidate.indirect_profile(column.time_step));
            }

        }
    }
    rstart.push_back(dmatval.size());

    solver_addrows(_math_problem, rowtype, rhs, {}, rstart, colind, dmatval);
}

void ProblemModifier::add_new_cost_constraints(const std::vector<ActiveLink> &active_links,
                              const std::map<linkId, ColumnsToChange> &p_cost_columns){

    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    for (const auto& link : active_links) {
        if (p_cost_columns.find(link._idLink) != p_cost_columns.end()) {
            for (auto column : p_cost_columns.at(link._idLink)) {
                rstart.push_back(dmatval.size());
                rhs.push_back(0);
                rowtype.push_back('L');
                colind.push_back(column.id);
                dmatval.push_back(1);
                for (const auto &candidate:link.getCandidates()) {
                    colind.push_back(_candidate_col_id[candidate._name]);
                    dmatval.push_back(-1);
                }
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
            int new_index = _candidate_col_id.size() + _n_cols_at_start;
            _candidate_col_id[candidate._name] = new_index;
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
