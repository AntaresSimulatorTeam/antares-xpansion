#include <antares-xpansion/presolve/presolve.h>
#include <filesystem>
#include <fmt/format.h>

#include "antares-xpansion/benders/benders_core/SimulationOptions.h"
#include "antares-xpansion/benders/benders_core/common.h"
#include "antares-xpansion/benders/logger/User.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/multisolver_interface/SolverXpress.h"

const std::string PRESOLVE_CONTEXT{"Presolve"};

int main(int argc, char** argv)
{
    usage(argc);
    PresolveOptions options{SimulationOptions(argv[1]).get_presolve_options()};
    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("Starting presolve", LogUtils::LOGLEVEL::INFO, PRESOLVE_CONTEXT);

    Presolve presolve;
    SolverAbstract::Ptr solver_ptr = presolve.init_solver(options, logger);

    presolve.reduce_problems(solver_ptr, options, logger);

    logger->display_message("Presolve finished", LogUtils::LOGLEVEL::INFO, PRESOLVE_CONTEXT);

    return 0;
}
