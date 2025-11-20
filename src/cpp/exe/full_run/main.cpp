#include <antares-xpansion/benders/factories/BendersApp.h>
#include <antares-xpansion/benders/logger/User.h>
#include <antares-xpansion/presolve/presolve.h>
#include <exception>
#include <iostream>

#include <boost/program_options.hpp>

#include "antares-xpansion/benders/benders_mpi/common_mpi.h"
#include "antares-xpansion/full_run/FullRunOptionsParser.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"
#include "antares-xpansion/study-updater/StudyUpdateRunner.h"
namespace po = boost::program_options;

void presolve(const std::filesystem::path& options_file)
{
    PresolveOptions options{SimulationOptions(options_file).get_presolve_options()};
    Logger logger = std::make_shared<xpansion::logger::User>(std::cout);

    logger->display_message("Starting presolve",
                            LogUtils::LOGLEVEL::INFO,
                            std::string(Presolve::PRESOLVE_CONTEXT));

    Presolve presolve;
    std::shared_ptr<SolverAbstract> solver_ptr = presolve.init_solver(options, logger);

    presolve.reduce_problems(solver_ptr, options, logger);

    logger->display_message("Presolve finished",
                            LogUtils::LOGLEVEL::INFO,
                            std::string(Presolve::PRESOLVE_CONTEXT));
}

int main(int argc, char** argv)
{
    mpi::environment env(argc, argv);
    mpi::communicator world;
    auto options_parser = FullRunOptionsParser();
    std::filesystem::path xpansion_output_dir;
    options_parser.Parse(argc, argv);
    if (world.rank() == 0)
    {
        try
        {
            ProblemGeneration pbg(options_parser);
            xpansion_output_dir = pbg.updateProblems();
            std::filesystem::copy_file(xpansion_output_dir / "area.txt",
                                       xpansion_output_dir / "lp" / "area.txt",
                                       std::filesystem::copy_options::overwrite_existing);
        }
        catch (std::exception& e)
        {
            std::cerr << "error: " << e.what() << std::endl;
            return 1;
        }
        catch (...)
        {
            std::cerr << "Exception of unknown type!" << std::endl;
        }
    }
    world.barrier();

    const auto options_file = options_parser.BendersOptionsFile();
    if (options_parser.presolve())
    {
        presolve(options_file);
    }

    auto solver = options_parser.Solver();
    if (solver == "benders")
    {
        auto benders_factory = BendersApp(options_file, world, SOLVER::BENDERS);
        benders_factory.Run();
    }
    if (solver == "adequacy_criterion")
    {
        auto benders_factory = BendersApp(options_file, world, SOLVER::OUTER_LOOP);
        benders_factory.Run();
    }
    return 0;
}
