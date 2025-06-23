
#include <iostream>

#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"

int main(int argc, char** argv)
{
    try
    {
        auto options_parser = ProblemGenerationExeOptions();
        options_parser.Parse(argc, argv);

        auto gridCollection = std::make_shared<GridCollection>(options_parser.StudyPath()
                                                               / "grid.csv");

        auto path_to_data = options_parser.StudyPath();
        auto report_path = path_to_data / "report.txt";
        auto logger_factory = FileAndStdoutLoggerFactory(report_path, false);
        Logger logger = logger_factory.get_logger();
        auto writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                           path_to_data / "output.json");

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;
        for (auto& grid: gridCollection->gridDefinitions)
        {
            ProblemGenerationForWaterValueCalculation pbg(options_parser, grid);
            auto mps_path = pbg.updateProblems();

            auto evaluator = GridEvaluator(logger,
                                           writer,
                                           mps_path,
                                           grid,
                                           ProblemsFormat::MPS_FILE,
                                           1);
            variationDeNiveauxDeStockData[grid.gridID] = evaluator.launch();
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
