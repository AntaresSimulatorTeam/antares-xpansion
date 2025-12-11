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