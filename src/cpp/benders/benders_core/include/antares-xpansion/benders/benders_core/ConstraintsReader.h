#pragma once 

#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <utility>


struct constraintRow 
{
    std::vector<int> mstart ; 
    std::vector<int> mclind ; 
    std::vector<double> dmatval ; 
    std::vector<double> range_p ; 
    std::vector<char> qrtype_p ; 
    std::vector<double> rhs ; 
    std::vector<std::string> row_names ; 
} ; 


class ConstraintsReader 
{
    public : 
    ConstraintsReader(const std::filesystem::path constraint_file_path, 
                     const std::string& solver_name, 
                     const SolverLogManager& solver_log_manager, 
                    Logger& logger, 
                    int log_level,
                    const std::filesystem::path variables_names_path,
                    const std::shared_ptr<SubproblemWorker>& subproblem_worker); 

    int get_row_index(const std::string& name ) ; 

    std::vector<std::pair<std::string,double>> get_variables_values() ; 

    constraintRow get_row(const std::string& name) ;
    std::shared_ptr<SubproblemWorker> get_subproblem_worker() ; 
    
    
    void get_variables_values_in_csv(std::filesystem::path variables_values_csv) ; 

    std::vector<double> get_sub_solution() ; 
    int get_variable_index_in_solution(std::string variable_name) ; 

    private : 
    Logger logger_ ; 
    std::shared_ptr<SolverAbstract> solver_ ; 
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_ ; 
    std::map<std::string,std::pair<std::string,int>> variables_names_map_ ; 
    std::shared_ptr<SubproblemWorker> subproblem_worker_ ; 
    SolverIO solver_IO_ ; 
} ; 


typedef std::shared_ptr<ConstraintsReader> ConstraintsReaderPtr ; 
typedef std::map<std::string,ConstraintsReaderPtr> ConstraintsReaderPtrMap ; 
