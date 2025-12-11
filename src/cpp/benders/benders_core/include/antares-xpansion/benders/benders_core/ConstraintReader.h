
#include "IBendersProblemProvider.h"
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include <antares-xpansion/benders/benders_core/SolverIO.h>

class ConstraintReader ; 
typedef std::shared_ptr<ConstraintReader> ConstraintReaderPtr ; 
typedef std::map<std::string,ConstraintReaderPtr> ConstraintReaderPtrMap ; 



class ConstraintReader 
{
    public : 
    ConstraintReader(const std::filesystem::path constraint_file_path, 
                     const std::string& solver_name, 
                     const SolverLogManager& solver_log_manager, 
                    Logger& logger, 
                    int log_level); 
 
    private : 
    Logger logger_ ; 
    std::shared_ptr<SolverAbstract> solver_ ; 
    std::shared_ptr<BendersProblemFromFile> benders_problem_provider_ ; 
    SolverIO solver_IO_ ; 
} ; 