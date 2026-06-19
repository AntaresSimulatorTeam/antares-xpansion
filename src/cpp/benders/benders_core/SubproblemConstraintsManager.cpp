#include "antares-xpansion/benders/benders_core/SubproblemConstraintsManager.h"

SubproblemConstraintsManager::SubproblemConstraintsManager(
  ConstraintsFileReader file_reader,
  const std::shared_ptr<SubproblemWorker>& subproblem_worker):
    file_reader_(std::move(file_reader)),
    subproblem_worker_(subproblem_worker),
    initial_sub_size_(subproblem_worker->get_problem_row_num())
{
}

std::vector<double> SubproblemConstraintsManager::get_sub_solution()
{
    if (subproblem_worker_ == nullptr) 
        std::cout<<"subproblem_worker_ is null "<<std::endl ; 
    return subproblem_worker_->get_solution();
}

int SubproblemConstraintsManager::get_variable_index_in_solution(std::string variable_name)
{
    return subproblem_worker_->get_variable_index(variable_name);
}

void SubproblemConstraintsManager::add_rows_to_subproblem(SolverRepresentedRows& new_row)
{
    subproblem_worker_->AddRows(new_row.qrtype_p,
                                new_row.rhs,
                                new_row.range_p,
                                new_row.mstart,
                                new_row.mclind,
                                new_row.dmatval,
                                new_row.row_names);
}

void SubproblemConstraintsManager::add_rows(std::string& row_name)
{
    auto constraint_row = file_reader_.get_row(row_name);
    add_rows_to_subproblem(constraint_row);
}

int SubproblemConstraintsManager::size_of_subproblem()
{
    return subproblem_worker_->get_problem_row_num();
}

void SubproblemConstraintsManager::delete_added_rows()
{
    subproblem_worker_->delete_rows(initial_sub_size_);
}
