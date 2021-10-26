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

void ProblemModifier::change_lower_bounds_to_neg_inf(const std::vector<int> &col_ids) const {
    std::vector<char> lb_char(col_ids.size(), 'L');
    std::vector<double> neg_inf(col_ids.size(), -1e20);
    _math_problem->chg_bounds(col_ids, lb_char, neg_inf);
}

void ProblemModifier::change_upper_bounds_to_pos_inf(const std::vector<int> &col_ids) const{
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

bool ProblemModifier::has_candidate_col_id(const std::string& cand_name) const{
    return _candidate_col_id.find(cand_name) != _candidate_col_id.end();
}

std::shared_ptr<SolverAbstract>
ProblemModifier::changeProblem(std::shared_ptr<SolverAbstract> mathProblem, const std::vector<ActiveLink> &active_links,
                               const std::map<linkId,  ColumnsToChange> &p_ntc_columns,
                               const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
                               const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns) {
    _math_problem = std::move(mathProblem);
    _n_cols_at_start = _math_problem->get_ncols();

    changeProblem(active_links, p_ntc_columns, p_direct_cost_columns,p_indirect_cost_columns);
    return std::move(_math_problem);
}

void ProblemModifier::changeProblem(const std::vector<ActiveLink> &active_links,
                                    const std::map<linkId, ColumnsToChange> &p_ntc_columns,
                                    const std::map<linkId, ColumnsToChange> &p_direct_cost_columns,
                                    const std::map<linkId, ColumnsToChange> &p_indirect_cost_columns) {
    for (const auto& link : active_links) {
        remove_bounds_for(extract_col_ids(p_ntc_columns.at(link.get_idLink())));

        if (p_direct_cost_columns.find(link.get_idLink()) != p_direct_cost_columns.end()) {
            change_upper_bounds_to_pos_inf(extract_col_ids(p_direct_cost_columns.at(link.get_idLink())));
        }
        if (p_indirect_cost_columns.find(link.get_idLink()) != p_indirect_cost_columns.end()) {
            change_upper_bounds_to_pos_inf(extract_col_ids(p_indirect_cost_columns.at(link.get_idLink())));
        }
    }

    add_new_columns(candidates_with_not_null_profile(active_links, extract_time_steps(p_ntc_columns)));

    add_new_ntc_constraints(active_links, p_ntc_columns);
    add_new_direct_cost_constraints(active_links, p_direct_cost_columns);
    add_new_indirect_cost_constraints(active_links, p_indirect_cost_columns);

}

void ProblemModifier::add_new_ntc_constraints(const std::vector<ActiveLink> &active_links,
                                              const std::map<linkId, ColumnsToChange> &p_ntc_columns) {
    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    rstart.push_back(0);

    for (const auto& link : active_links) {
        for(const ColumnToChange& column : p_ntc_columns.at(link.get_idLink())){
            add_direct_profile_column_constraint(dmatval,colind,rowtype,rhs,rstart,link,column);
            add_indirect_profile_ntc_column_constraint(dmatval, colind, rowtype, rhs, rstart, link, column);
        }
    }

    solver_addrows(_math_problem, rowtype, rhs, {}, rstart, colind, dmatval);
}

void ProblemModifier::add_new_direct_cost_constraints(const std::vector<ActiveLink> &active_links,
                              const std::map<linkId, ColumnsToChange> &p_cost_columns){

    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    rstart.push_back(0);

    for (const ActiveLink& link : active_links) {
        if (p_cost_columns.find(link.get_idLink()) != p_cost_columns.end()) {
            for (const ColumnToChange& column : p_cost_columns.at(link.get_idLink())) {
                add_direct_profile_column_constraint(dmatval, colind, rowtype, rhs, rstart, link, column);
            }
        }
    }

    solver_addrows(_math_problem, rowtype, rhs, {}, rstart, colind, dmatval);

}


void ProblemModifier::add_new_indirect_cost_constraints(const std::vector<ActiveLink> &active_links,
                                                      const std::map<linkId, ColumnsToChange> &p_cost_columns){

    std::vector<double> dmatval;
    std::vector<int> colind;
    std::vector<char> rowtype;
    std::vector<double> rhs;
    std::vector<int> rstart;

    rstart.push_back(0);

    for (const auto& link : active_links) {
        if (p_cost_columns.find(link.get_idLink()) != p_cost_columns.end()) {
            for (const ColumnToChange& column : p_cost_columns.at(link.get_idLink())) {
                add_indirect_cost_column_constraint(dmatval, colind, rowtype, rhs, rstart, link, column);
            }
        }
    }

    solver_addrows(_math_problem, rowtype, rhs, {}, rstart, colind, dmatval);
}

void ProblemModifier::add_direct_profile_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                                           std::vector<char> &rowtype, std::vector<double> &rhs,
                                                           std::vector<int> &rstart, const ActiveLink &link,
                                                           const ColumnToChange &column) {
    double already_installed_capacity(link.get_already_installed_capacity());
    double direct_already_installed_profile_at_timestep = link.already_installed_direct_profile(column.time_step);
    rhs.push_back(already_installed_capacity * direct_already_installed_profile_at_timestep);

    rowtype.push_back('L');
    colind.push_back(column.id);
    dmatval.push_back(1);
    for (const auto &candidate:link.getCandidates()) {
        if (candidate.direct_profile(column.time_step) != 0.0) {
            colind.push_back(_candidate_col_id[candidate.get_name()]);
            dmatval.push_back(-candidate.direct_profile(column.time_step));
        }
    }
    rstart.push_back( (int) dmatval.size());
}

void ProblemModifier::add_indirect_profile_ntc_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                                                 std::vector<char> &rowtype, std::vector<double> &rhs,
                                                                 std::vector<int> &rstart, const ActiveLink &link,
                                                                 const ColumnToChange &column) {
    double already_installed_capacity(link.get_already_installed_capacity());
    double indirect_already_installed_profile_at_timestep = link.already_installed_indirect_profile(column.time_step);
    rhs.push_back(-already_installed_capacity*indirect_already_installed_profile_at_timestep);

    rowtype.push_back('G');
    colind.push_back(column.id);
    dmatval.push_back(1);
    for (const auto &candidate:link.getCandidates()) {
        if (candidate.indirect_profile(column.time_step) != 0.0) {
            colind.push_back(_candidate_col_id[candidate.get_name()]);
            dmatval.push_back(candidate.indirect_profile(column.time_step));
        }
    }
    rstart.push_back((int) dmatval.size());
}

void ProblemModifier::add_indirect_cost_column_constraint(std::vector<double> &dmatval, std::vector<int> &colind,
                                                          std::vector<char> &rowtype, std::vector<double> &rhs,
                                                          std::vector<int> &rstart, const ActiveLink &link,
                                                          const ColumnToChange &column) {
    double already_installed_capacity(link.get_already_installed_capacity());
    double indirect_already_installed_profile_at_timestep = link.already_installed_indirect_profile(column.time_step);
    rhs.push_back(already_installed_capacity*indirect_already_installed_profile_at_timestep);

    rowtype.push_back('L');
    colind.push_back(column.id);
    dmatval.push_back(1);
    for (const auto &candidate:link.getCandidates()) {
        if (candidate.indirect_profile(column.time_step) != 0.0) {
            colind.push_back(_candidate_col_id[candidate.get_name()]);
            dmatval.push_back(-candidate.indirect_profile(column.time_step));
        }
    }
    rstart.push_back((int) dmatval.size());
}

void ProblemModifier::add_new_columns(const std::vector<Candidate> &candidates) {
    if(!candidates.empty()){
        unsigned int n_candidates = (int) candidates.size();
        std::vector<double> objectives(n_candidates, 0);
        std::vector<double> lb(n_candidates, -1e20);
        std::vector<double> ub(n_candidates, 1e20);
        std::vector<char> coltypes(n_candidates, 'C');
        std::vector<int> mstart(n_candidates, 0);
        std::vector<std::string> candidates_colnames;

        for (const auto& candidate : candidates) {
            candidates_colnames.push_back(candidate.get_name());
            int new_index = (int)_candidate_col_id.size() + (int) _n_cols_at_start;
            _candidate_col_id[candidate.get_name()] = new_index;
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
std::set<int> ProblemModifier::extract_time_steps(const std::map<linkId, ColumnsToChange> &p_columns) const{
    std::set<int> result;
    for (const auto& columns : p_columns){
        for(const auto& column : columns.second){
            result.insert(column.time_step);
        }
    }
    return result;
}

std::vector<Candidate> ProblemModifier::candidates_with_not_null_profile(const std::vector<ActiveLink> &active_links, const std::set<int>& time_steps) const{
    std::vector<Candidate> candidates = candidates_from_all_links(active_links);

    candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [time_steps](const Candidate& cand) {
        bool hasOnlyNullProfile = true;
        for (int time_step : time_steps) {
            hasOnlyNullProfile &= cand.direct_profile(time_step) == 0.0;
            hasOnlyNullProfile &= cand.indirect_profile(time_step) == 0.0;
        }
        return hasOnlyNullProfile;
    }), candidates.end());

    return candidates;
}
