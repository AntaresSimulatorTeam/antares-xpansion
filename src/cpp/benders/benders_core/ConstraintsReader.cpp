#include "antares-xpansion/benders/benders_core/ConstraintsReader.h"

#include "antares-xpansion/benders/plugins/Benders_Jl_MICRO_ITERS.h"


#include <boost/tokenizer.hpp>

ConstraintsReader::ConstraintsReader(const std::filesystem::path constraint_file_path,
                                     const std::string& solver_name,
                                     const SolverLogManager& solver_log_manager,
                                     Logger& logger,
                                     int log_level,
                                     const std::filesystem::path variables_names_path,
                                     const std::shared_ptr<SubproblemWorker>& subproblem_worker):
    logger_(logger)

{
    SolverFactory solver_factory(logger_);
    subproblem_worker_ = subproblem_worker;
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);
    if (solver_)
    {
        solver_->set_threads(1);
        solver_->set_output_log_level(log_level);
        benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(constraint_file_path);
        solver_IO_.configure(solver_name, ProblemsFormat::MPS_FILE);
        benders_problem_provider_->provide_problem(solver_IO_, solver_);
        int n_rows = solver_->get_nrows();
    }

    std::ifstream variables_file(variables_names_path);
    if (variables_file.is_open())
    {
        std::string line;
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;

        while (std::getline(variables_file, line))
        {
            Tokenizer tok(line);
            std::vector<std::string> tokens(tok.begin(), tok.end());
            int variable_index = subproblem_worker->get_variable_index(tokens[1]);
            variables_names_map_[tokens[0]] = std::make_pair(tokens[1], variable_index);
        }
    }
    else
    {
        std::cerr << "variables file is not opened" << std::endl;
    }
}

std::shared_ptr<SubproblemWorker> ConstraintsReader::get_subproblem_worker()
{
    return subproblem_worker_;
}

int ConstraintsReader::get_row_index(const std::string& name)
{
    int row_pos(-1);
    if (solver_)
    {
        row_pos = solver_->get_row_index(name);
    }
    return row_pos;
}

constraintRow ConstraintsReader::get_row(const std::string& name)
{
    constraintRow result;
    result.range_p = {};
    result.row_names = {name};
    int constraint_pos = get_row_index(name);
    if (solver_)
    {
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
    }
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

void ConstraintsReader::add_rows_to_subproblems(constraintRow& new_row)
{
    // std::cout<<"adding row  to subproblem !! "<<std::endl ;
    // std::cout<<"qrtype_p size "<<new_row.qrtype_p.size()<<" new_row.rhs "<<new_row.rhs.size()<<
    // " new_row.range_p size "<<new_row.range_p.size()<<"  new_row.mstart size
    // "<<new_row.mstart.size()<<std::endl ;
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


void ConstraintsReader::delete_added_rows(std::vector<std::string>& added_rows)
{
    for (auto& added_row : added_rows) 
    {
        subproblem_worker_->delete_row(added_row) ; 
    }
} 




