#pragma once

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <utility>

#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"


/*
    The structure contains the necesary vector to fetch for a row of an mps file
*/
struct SolverRepresentedRows
{
    std::vector<int> mstart;
    std::vector<int> mclind;
    std::vector<double> dmatval;
    std::vector<double> range_p;
    std::vector<char> qrtype_p;
    std::vector<double> rhs;
    std::vector<std::string> row_names;
};

/*
    This class is a sort of wrapper around solverAbstract. 
    It allows reading the mps constraints files and provides 
    the necessary methods in our use case
*/
class ConstraintsReader
{
public:

    /*
        Constructor 
        @inputs : 
            - constraint_file_path : path to the mps constraint file
            - solver_name : solver name 
            - solver_log_manager : solver log manager 
            - logger : benders logger 
            - log_levl : logging level 
            - subproblem_worker : worker associated to the subproblem
    */
    ConstraintsReader(const std::filesystem::path constraint_file_path,
                      const std::string& solver_name,
                      const SolverLogManager& solver_log_manager,
                      Logger& logger,
                      int log_level,
                      const std::shared_ptr<SubproblemWorker>& subproblem_worker);



    void add_rows(std::string&);
    std::vector<double> get_sub_solution();
    int get_variable_index_in_solution(std::string variable_id);
    void delete_added_rows(std::vector<std::string>&) ; 
    
    private:
    
    void add_rows_to_subproblems(SolverRepresentedRows&);
    int get_row_index(const std::string& name);
    SolverRepresentedRows get_row(const std::string& name);
    std::shared_ptr<SubproblemWorker> get_subproblem_worker();
    void get_variables_values_in_csv(std::filesystem::path variables_values_csv);


    Logger logger_;
    std::shared_ptr<SolverAbstract> solver_;
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_;
    std::shared_ptr<SubproblemWorker> subproblem_worker_;
    SolverIO solver_IO_;
};

typedef std::shared_ptr<ConstraintsReader> ConstraintsReaderPtr;
typedef std::map<std::string, ConstraintsReaderPtr> ConstraintsReaderPtrMap;
