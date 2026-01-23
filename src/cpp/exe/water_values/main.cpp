
#include <cerrno>
#include <chrono>
#include <iostream>

#include "antares-xpansion/bellman_values/BellmanValues.h"
#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"
#include "antares-xpansion/bellman_values/PenaltiesConfigReader.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

std::vector<double> interpolateVector(const std::vector<double>& originalValues, int targetSize)
{
    std::vector<double> result(targetSize);
    int originalSize = static_cast<int>(originalValues.size());

    if (originalSize == 0 || targetSize == 0)
    {
        return result;
    }

    // Cas particulier : un seul point
    if (originalSize == 1)
    {
        std::fill(result.begin(), result.end(), originalValues[0]);
        return result;
    }

    for (int i = 0; i < targetSize; ++i)
    {
        // Position correspondante dans le vecteur d'origine
        double positionInOriginal = static_cast<double>(i) * (originalSize - 1) / (targetSize - 1);

        // Indices entourant cette position
        int lowerIndex = static_cast<int>(std::floor(positionInOriginal));
        int upperIndex = std::min(lowerIndex + 1, originalSize - 1);

        double fraction = positionInOriginal - lowerIndex;

        // Interpolation linéaire
        double interpolated = (1.0 - fraction) * originalValues[lowerIndex]
                              + fraction * originalValues[upperIndex];

        result[i] = interpolated;
    }

    return result;
}

std::vector<std::vector<double>> interpolateWeekVector(
  const std::vector<std::vector<double>>& originalValues,
  int targetSize)
{
    std::vector<std::vector<double>> interpolatedValues;
    for (const auto& weekValues: originalValues)
    {
        interpolatedValues.push_back(interpolateVector(weekValues, targetSize));
    }
    return interpolatedValues;
}

std::string formatTime(const std::chrono::system_clock::time_point& timePoint)
{
    // the <format> STL seems to not be available on all used compilers yet
    std::time_t tt = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tm = *std::localtime(&tt); // Locale time-zone, usually UTC by default.
    return (std::stringstream() << std::put_time(&tm, "%T")).str();
}

template<typename T>
std::string formatDuration(std::chrono::duration<T> duration)
{
    // the <format> STL seems to not be available on all used compilers yet
    auto h = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= h;
    auto m = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= m;
    auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
    return (std::stringstream() << h.count() << "h, " << m.count() << "m, " << s.count() << "s")
      .str();
}

void saveValues(const std::filesystem::path& path,
                const std::vector<std::vector<double>>& values,
                const Logger& logger,
                bool usingAntaresFormat = false)
{
    std::ofstream file(path);
    if (!file)
    {
        // std::cerr << "Failed to open file: " << path << std::endl;
        logger->display_message("Failed to open file: " + path.string(),
                                LogUtils::LOGLEVEL::ERR,
                                "Water Values");
        logger->display_message("Error opening file: " + std::string(std::strerror(errno)),
                                LogUtils::LOGLEVEL::ERR,
                                "Water Values");
        return;
    }

    if (usingAntaresFormat)
    {
        // padding with a line of 0 for each level
        for (int levelIdx = 0; levelIdx < values[0].size(); ++levelIdx)
        {
            file << "0\t";
        }
        file << '\n';
    }
    for (const auto& weekValues: values)
    {
        std::vector<double> values = weekValues;
        if (usingAntaresFormat)
        {
            for (size_t i = 0; i < 7; i++)
            {
                for (const auto& value: values)
                {
                    file << value << '\t';
                }
                file << 0;
                if (i != 6)
                {
                    file << '\n';
                }
            }
        }
        else
        {
            for (const auto& value: values)
            {
                file << value << " ";
            }
        }
        file << '\n';
    }
}

std::vector<std::vector<double>> computeWaterValues(
  const std::vector<std::vector<double>>& bellmanValues,
  const std::vector<double>& levels)
{
    if (bellmanValues.empty())
    {
        return {};
    }

    size_t numLevels = levels.size();
    size_t numWeeks = bellmanValues.size() - 1;

    for (const auto& weekVals: bellmanValues)
    {
        if (weekVals.size() != numLevels)
        {
            throw std::invalid_argument("Inconsistent level size in bellmanValues");
        }
    }

    std::vector<std::vector<double>> derivatives(numWeeks, std::vector<double>(numLevels - 1));

    for (size_t week = 1; week <= numWeeks; ++week)
    {
        const auto& values = bellmanValues[week];
        for (size_t i = 0; i < numLevels - 1; ++i)
        {
            // Take the opposite of the derivative to have positive water values
            derivatives[week - 1][i] = -(values[i + 1] - values[i]) / (levels[i + 1] - levels[i]);
        }
    }

    return derivatives;
}

int main(int argc, char** argv)
{
    try
    {
        auto optionsParser = BellmanValuesExeOptions();
        optionsParser.Parse(argc, argv);
        auto studyPath = optionsParser.StudyPath();
        auto solverName = optionsParser.SolverName();
        int nbThreads = optionsParser.NbThreads();
        int startWeek = optionsParser.StartWeek();
        int endWeek = optionsParser.EndWeek();
        int nbLevels = optionsParser.NbLevels();
        bool antaresFormat = optionsParser.AntaresFormat();
        bool writePbFiles = optionsParser.WritePbFiles();
        const std::string problemFormat = optionsParser.ProblemFormat();
        const bool ignoreOptimalTrajectory = optionsParser.IgnoreOptimalTrajectory();

        auto gridCollection = std::make_shared<GridCollection>(studyPath
                                                               / "user/water_values/grid.csv");

        const std::filesystem::path penaltiesConfigFilePath(studyPath
                                                            / "user/water_values/penalties.yaml");

        // PenaltiesConfigReader will check whether the file exists and return default values if
        // needed
        PenaltiesConfigReader pcr(penaltiesConfigFilePath);

        ConfigurationManager::ConfigDirectories directories{
          .study_dir = studyPath,
          .simulation_dir = ConfigurationManager::generateOutputName(studyPath),
        };

        // at this point, the simulation folder is already needed for logs (normally created when
        // updating problems)
        if (!std::filesystem::exists(directories.simulation_dir))
        {
            std::filesystem::create_directories(directories.simulation_dir);
        }
        std::filesystem::path logPath = directories.simulation_dir / "water_values_log.txt";
        std::ofstream{logPath}; // creates log file, since the FileLoggerFactory doesn't
        auto loggerFactory = FileAndStdoutLoggerFactory(logPath, false);
        Logger logger = loggerFactory.get_logger();

        auto startProblemGeneration = std::chrono::system_clock::now();
        logger->display_message(
          "Generating problems (starting time: " + formatTime(startProblemGeneration) + ")");
        ProblemGenerationForWaterValueCalculation pbg(
          directories,
          logger,
          solverName,
          ProblemGenerationForWaterValueCalculation::getComputationModeFromGrid(
            *gridCollection,
            ignoreOptimalTrajectory),
          startWeek,
          endWeek,
          writePbFiles,
          problemFormat);
        auto endProblemGeneration = std::chrono::system_clock::now();
        logger->display_message("Problems generated (end time: " + formatTime(endProblemGeneration)
                                + ")");
        std::chrono::duration<double> elapsed_seconds = endProblemGeneration
                                                        - startProblemGeneration;
        logger->display_message("Elapsed time for problem generation: "
                                + formatDuration(elapsed_seconds));

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;

        // modify all reservoirs for the grid collection here with optimal trajectories
        if (pbg.getComputationMode()
              == ProblemGenerationForWaterValueCalculation::WaterValueComputationMode::
                SEQUENTIAL_UPDATE_TRAJECTORY
            || pbg.getComputationMode()
                 == ProblemGenerationForWaterValueCalculation::WaterValueComputationMode::
                   SEQUENTIAL_IGNORE_TRAJECTORY)
        {
            pbg.initializeOptimalTrajectories(gridCollection);
        }

        // here: loop on all grids
        for (auto& grid: gridCollection->gridDefinitions | std::views::values)
        {
            logger->display_message("## GridDefinition ID: " + std::to_string(grid.gridID) + " ##");

            // hypothesis: all gridElements are linked to a reservoir.
            // In the case of multistock, there should only be one reservoir per
            // gridDefinition; in the case of multivariate, there would be more than one
            // reservoir per gridDefinition
            grid.setReservoirs(gridCollection->reservoirs);

            for (auto& gridElement: grid.gridElements)
            {
                logger->display_message("### Grid element area: " + gridElement.area + " ###");
                // multistock here
                // update the reservoir in ReservoirManagement based on the considered area
                ReservoirManagement reservoirManagement(grid.reservoirs.at(gridElement.area),
                                                        pcr.getPenaltyBottomRuleCurve(),
                                                        pcr.getPenaltyUpperRuleCurve(),
                                                        pcr.getPenaltyFinalLevel(),
                                                        pcr.getForceFinalLevel(),
                                                        pcr.getFinalLevel(),
                                                        pcr.getCvar());
                // this is also where we will update penalties if they need to be

                if (reservoirManagement.reservoir.area != gridElement.area)
                {
                    reservoirManagement.setReservoir(
                      gridCollection->reservoirs.at(gridElement.area));
                }
                auto startProblemUpdate = std::chrono::system_clock::now();
                logger->display_message(
                  "Updating problems (starting time: " + formatTime(startProblemUpdate) + ")");

                auto problems = pbg.updateProblems(grid, reservoirManagement, gridElement.area);

                auto endProblemUpdate = std::chrono::system_clock::now();
                logger->display_message(
                  "Updated problems (end time: " + formatTime(endProblemUpdate) + ")");

                std::chrono::duration<double> elapsed_update_seconds = endProblemUpdate
                                                                       - startProblemUpdate;
                logger->display_message("Elapsed time for problem update: "
                                        + formatDuration(elapsed_update_seconds));

                logger->display_message("Instantiating GridEvaluator");
                auto evaluator = GridEvaluator(logger,
                                               problems,
                                               grid,
                                               solverName,
                                               directories.simulation_dir,
                                               nbThreads);

                logger->display_message("Instantiating BellmanValues");
                auto bellmanValuesEvaluator = BellmanValues(evaluator, reservoirManagement, logger);

                logger->display_message("Computing Bellman values...");
                auto bellmanValues = bellmanValuesEvaluator.compute(nbLevels);
                logger->display_message("Computed Bellman values");

                std::string bellmanValuesFileName = std::to_string(grid.gridID) + "_"
                                                    + gridElement.area + "_bellman_values.csv";
                saveValues(directories.simulation_dir / bellmanValuesFileName,
                           bellmanValues,
                           logger,
                           false);

                auto levels = bellmanValuesEvaluator.getLevels();
                if (antaresFormat)
                {
                    bellmanValues = interpolateWeekVector(bellmanValues, 101);
                    levels = interpolateVector(levels, 101);
                }

                logger->display_message("Computing water values...");
                auto waterValues = computeWaterValues(bellmanValues, levels);
                logger->display_message("Computed water values");

                std::string fileName = std::to_string(grid.gridID) + "_" + gridElement.area
                                       + "_water_values.csv";
                saveValues(directories.simulation_dir / fileName,
                           waterValues,
                           logger,
                           antaresFormat);
                logger->display_message("Saved water values to file");

                gridCollection->reservoirs.at(gridElement.area) = reservoirManagement.reservoir;

                if (pbg.getComputationMode()
                    == ProblemGenerationForWaterValueCalculation::WaterValueComputationMode::
                      SEQUENTIAL_UPDATE_TRAJECTORY)
                {
                    logger->display_message("Computing optimal trajectory...");

                    gridCollection->reservoirs.at(gridElement.area).optimal_trajectory
                      = bellmanValuesEvaluator.computeOptimalTrajectories();

                    logger->display_message("Computed optimal trajectory");

                    std::string optimalTrajectoriesFileName = std::to_string(grid.gridID) + "_"
                                                              + gridElement.area
                                                              + "_optimal_trajectory.csv";
                    saveValues(directories.simulation_dir / optimalTrajectoriesFileName,
                               gridCollection->reservoirs.at(gridElement.area).optimal_trajectory,
                               logger,
                               false);
                }
            }
        }
        logger->display_message("Computing water values: Done!");

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
        return 1;
    }

    return 0;
}
