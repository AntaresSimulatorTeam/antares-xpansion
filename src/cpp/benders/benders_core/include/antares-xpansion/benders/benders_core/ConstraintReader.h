
#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include <antares-xpansion/benders/benders_core/SolverIO.h>

class ConstraintReader ; 
typedef std::shared_ptr<ConstraintReader> ConstraintReaderPtr ; 
typedef std::map<std::string,ConstraintReaderPtr> ConstraintReaderPtrMap ; 


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


class ConstraintReader 
{
    public : 
    ConstraintReader(const std::filesystem::path constraint_file_path, 
                     const std::string& solver_name, 
                     const SolverLogManager& solver_log_manager, 
                    Logger& logger, 
                    int log_level); 

    int get_row_index(const std::string& name ) ; 

    constraintRow get_row(const std::string& name) ;
 
    private : 
    Logger logger_ ; 
    std::shared_ptr<SolverAbstract> solver_ ; 
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_ ; 
    SolverIO solver_IO_ ; 
} ; 