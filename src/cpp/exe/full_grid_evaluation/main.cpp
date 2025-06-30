
#include <iostream>

#include <antares/api/solver.h>

#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/grid_evaluator/ReservoirManagement.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> generateProblems(
  std::filesystem::path studyPath,
  std::filesystem::path outputPath)
{
    Output::ConcurrentInsertionMap<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>>
      problems;
    Antares::Solver::Optimization::OptimizationOptions optOptions;
    optOptions.linearSolver = "coin";

    auto [results, error] = Antares::API::PerformSimulation(studyPath, outputPath, optOptions);

#ifndef _WIN32
    malloc_trim(0);
#endif

    // Handle errors
    if (error)
    {
        throw LogUtils::XpansionError<std::runtime_error>("Antares simulation failed:\n\t"
                                                            + error->reason,
                                                          LOGLOCATION);
    }

    XpansionProblemsFromAntaresProvider adapter(results);
    for (const auto& [pbId, _]: results.weeklyProblems)
    {
        auto solver_log_manager = SolverLogManager(outputPath / "solver.log");
        auto problem = adapter.provideProblem("CBC", solver_log_manager, pbId);
        problems.insert(pbId, problem);
    }
    return problems.get();
}

int main(int argc, char** argv)
{
    try
    {
        auto options_parser = ProblemGenerationExeOptions();
        options_parser.Parse(argc, argv);

        auto gridCollection = std::make_shared<GridCollection>(options_parser.StudyPath()
                                                               / "grid.csv");

        Reservoir reservoir(options_parser.StudyPath(), "area");
        ReservoirManagement reservoir_management(reservoir, true);

        ConfigurationManager configuration_manager(options_parser);
        auto path_to_data = options_parser.StudyPath();
        auto report_path = path_to_data / "report.txt";
        auto logger_factory = FileAndStdoutLoggerFactory(report_path, false);
        Logger logger = logger_factory.get_logger();
        auto writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                           path_to_data / "output.json");

        std::map<Antares::Solver::WeeklyProblemId, std::shared_ptr<Problem>> problems;
        problems = generateProblems(path_to_data,
                                    configuration_manager.Directories().simulation_dir);

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;
        for (auto& grid: gridCollection->gridDefinitions)
        {
            ProblemGenerationForWaterValueCalculation pbg(options_parser, problems, grid);
            auto mps_path = pbg.updateProblems();

            auto evaluator = GridEvaluator(logger,
                                           writer,
                                           mps_path,
                                           grid,
                                           reservoir_management,
                                           ProblemsFormat::MPS_FILE,
                                           1);
            variationDeNiveauxDeStockData[grid.gridID] = evaluator.ComputeRewards();

            evaluator.ComputeBellmanValues();
        }

        writer->write_VariationDeNiveauxDeStock(variationDeNiveauxDeStockData);
        writer->dump();

        return 0;
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

    return 0;
}
