#include "antares-xpansion/benders/benders_core/ConstraintReader.h"

ConstraintReader::ConstraintReader(const std::filesystem::path constraint_file_path, 
                     const std::string& solver_name, 
                     const SolverLogManager& solver_log_manager, 
                    Logger& logger,
                    int log_level) : 
    logger_(logger)

{

    SolverFactory solver_factory(logger_) ;
    solver_ = solver_factory.create_solver(solver_name,SOLVER_TYPE::CONTINUOUS,solver_log_manager) ; 
    if (solver_)
    {
        solver_->set_threads(1) ; 
        solver_->set_output_log_level(log_level);
        benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(constraint_file_path) ; 
        solver_IO_.configure(solver_name,ProblemsFormat::MPS_FILE) ; 
        benders_problem_provider_->provide_problem(solver_IO_,solver_) ;
        int n_rows = solver_->get_nrows() ; 
        std::cout<<"number of rows in constraint problem "<<n_rows<<std::endl ;  

    }




}


int ConstraintReader::get_row_index(const std::string& name ) 
{
    int row_pos(-1) ; 
    if (solver_) 
    {
        row_pos = solver_->get_row_index(name) ; 
    }
    return row_pos ; 
}

constraintRow ConstraintReader::get_row(const std::string& name) 
{
    constraintRow result ; 
    result.range_p = {} ; 
    result.row_names = {name} ; 
    int constraint_pos = get_row_index(name) ; 
    if (solver_) 
    {
        int ncols = solver_->get_ncols() ; 
        result.mstart.resize(ncols,0) ; 
        result.mclind.resize(ncols,0) ; 
        result.dmatval.resize(ncols,0) ; 
        std::vector<int> nels ; 
        nels.resize(ncols,0) ; 

        solver_->get_rows(result.mstart.data(), 
                          result.mclind.data(), 
                          result.dmatval.data(), 
                          ncols, 
                         nels.data(), 
                         constraint_pos,
                         constraint_pos) ; 

        result.mstart.resize(2) ;
        result.mclind.resize(result.mstart[1]) ; 
        result.dmatval.resize(result.mstart[1]) ;   

        double rhs ; 
        solver_->get_rhs(&rhs,constraint_pos,constraint_pos) ; 
        result.rhs = {rhs} ; 

        const int MAX_LEN = 10;          
        char buffer[MAX_LEN];  
        solver_->get_row_type(buffer,constraint_pos,constraint_pos) ; 
        int len = 0;
        while (len < MAX_LEN && buffer[len] >= 'A' && buffer[len] <= 'Z') {
            ++len;
        }
        std::string qrtype(buffer, len);
        result.qrtype_p = {qrtype[0]} ; 

    }
    return result ; 
}