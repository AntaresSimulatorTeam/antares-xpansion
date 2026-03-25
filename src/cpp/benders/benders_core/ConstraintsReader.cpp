#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"

#include <boost/tokenizer.hpp>

ConstraintsReader::ConstraintsReader(const std::filesystem::path constraint_file_path,
                                     const std::string& solver_name,
                                     const SolverLogManager& solver_log_manager,
                                     Logger& logger,
                                     int log_level,
                                     ProblemsFormat format,
                                     const std::shared_ptr<SubproblemWorker>& subproblem_worker):
    logger_(logger)

{
    auto constraints_file_path_str = std::string(constraint_file_path.c_str());
    auto pos = constraints_file_path_str.find('.');
    auto extension = constraints_file_path_str.substr(pos + 1);

    SolverFactory solver_factory(logger_);
    subproblem_worker_ = subproblem_worker;
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_->set_threads(1);
    solver_->set_output_log_level(log_level);

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(constraint_file_path);
    solver_IO_.configure(solver_name, format);
    benders_problem_provider_->provide_problem(solver_IO_, solver_);
    int n_rows = solver_->get_nrows();
    initial_sub_size = subproblem_worker->get_problem_row_num();
}

std::shared_ptr<SubproblemWorker> ConstraintsReader::get_subproblem_worker()
{
    return subproblem_worker_;
}

int ConstraintsReader::get_row_index(const std::string& name)
{
    int row_pos(-1);
    row_pos = solver_->get_row_index(name);
    return row_pos;
}

SolverRepresentedRows ConstraintsReader::get_row(const std::string& name)
{
    SolverRepresentedRows result;
    result.range_p = {};
    result.row_names = {name};
    int constraint_pos = get_row_index(name);

    int ncols = solver_->get_ncols();
    result.mstart.resize(2);
    result.mclind.resize(ncols);
    result.dmatval.resize(ncols);

    int nels(0);

    solver_->get_rows(result.mstart.data(),
                      result.mclind.data(),
                      result.dmatval.data(),
                      ncols,
                      &nels,
                      constraint_pos,
                      constraint_pos);

    result.mclind.resize(nels);
    result.dmatval.resize(nels);
    result.mstart.resize(1);

    double rhs(0.);
    solver_->get_rhs(&rhs, constraint_pos, constraint_pos);
    result.rhs = {rhs};

    double range_p(0.);
    solver_->get_rhs_range(&range_p, constraint_pos, constraint_pos);
    result.range_p = {range_p};

    const int MAX_LEN = 10;
    char buffer[MAX_LEN];
    solver_->get_row_type(buffer, constraint_pos, constraint_pos);
    int len = 0;
    while (len < MAX_LEN && buffer[len] >= 'A' && buffer[len] <= 'Z')
    {
        ++len;
    }
    std::string qrtype(buffer, len);
    result.qrtype_p = {qrtype[0]};

    return result;
}

std::vector<double> ConstraintsReader::get_sub_solution()
{
    return subproblem_worker_->get_solution();
}

int ConstraintsReader::get_variable_index_in_solution(std::string variable_name)
{
    int variable_index(-1);
    variable_index = subproblem_worker_->get_variable_index(variable_name);
    return variable_index;
}

void ConstraintsReader::add_rows_to_subproblems(SolverRepresentedRows& new_row)
{
    subproblem_worker_->AddRows(new_row.qrtype_p,
                                new_row.rhs,
                                new_row.range_p,
                                new_row.mstart,
                                new_row.mclind,
                                new_row.dmatval,
                                new_row.row_names);
}

void ConstraintsReader::add_rows(std::string& row_name)
{
    auto constraint_row = get_row(row_name);
    add_rows_to_subproblems(constraint_row);
}

int ConstraintsReader::size_of_subproblem()
{
    return subproblem_worker_->get_problem_row_num();
}

void ConstraintsReader::delete_added_rows()
{
    subproblem_worker_->delete_rows(initial_sub_size);
}
