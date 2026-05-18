#pragma once 
#include <memory>
#include <string>
#include <vector>
#include <filesystem>


#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"

#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/tokenizer.hpp>




/*
This class will handle reading the constraints file in case of memory optimization 
The main differnce compared to the classical case is that when we are in memory optimization mode
we don't have a seperate mps per constraints : we have a skeleton mps file and some csv files that allows to read 
set the solver abstract into the right sub problem 
*/
class ConstraintsFileReaderMemOptim
{
    public : 
        ConstraintsFileReaderMemOptim(const std::filesystem::path &inputRoot,
                              const std::string& solver_name,
                              const SolverLogManager& solver_log_manager,
                              Logger& logger,
                              int log_level,
                              ProblemsFormat format
                               ) ; 

    
    private : 
        //as the input for memory optimization changes, with these method we read the necessary files 
        //to construct the correspondant constraint solver abstract   
        void read_coef(); 
        void read_coef_cols();
        void read_coef_rows();
        void read_obj_coef();
        void read_obj_cols();
        void read_rhs();
        void read_rhs_rows();
        void read_constraints_for_mem_optim() ;
        void build_skeleton(std::string solver_name,
                            const SolverLogManager& solver_log_manager,
                            int log_level,
                            ProblemsFormat format) ;  

        std::map<std::string, std::vector<double>> coeffs_;
        std::vector<std::string> coef_cols_;
        std::vector<std::string> coef_rows_;
        std::map<std::string, std::vector<double>> obj_coefs_;
        std::vector<std::string> obj_cols_;
        std::vector<std::string> rhs_rows_;
        std::map<std::string, std::vector<double>> rhs_;
        std::vector<int> constraints_col_indices_;
        std::vector<int> constraints_row_indices_;
        std::vector<int> obj_col_indices_;
        std::vector<int> rhs_row_indices_;


        Logger logger_;
        std::filesystem::path inputRoot_ ; 
        std::shared_ptr<SolverAbstract> solver_skeleton;
        std::shared_ptr<BendersProblemFromFile> benders_problem_provider_;
        SolverIO solver_IO_;
} ; 