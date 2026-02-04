
#include <chrono>
#include <iostream>
#include <ranges>

#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"
#include "antares-xpansion/bellman_values/PenaltiesConfigReader.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/logger/FilteredLogger.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

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

// C++ 23 : use std::views::join_with
template<std::ranges::input_range R>
void write_joined(std::ostream& os, R&& r, std::string_view sep)
{
    auto it = std::begin(r);
    auto end = std::end(r);

    if (it == end)
    {
        return;
    }

    os << *it;
    ++it;

    for (; it != end; ++it)
    {
        os << sep << *it;
    }
}

void saveCostsAndDuals(const std::filesystem::path& path,
                       const GridDefinition grid,
                       const std::map<Output::PointWeekScenarioKey, GridPointResult>& values,
                       const Logger& logger)
{
    std::ofstream file(path);
    if (!file)
    {
        logger->display_message("Failed to open file: " + path.string(),
                                LogUtils::LOGLEVEL::ERR,
                                "Water Values");
        return;
    }

    file << std::setprecision(std::numeric_limits<double>::max_digits10);

    // Header
    file << "scenario,week,";
    std::set<std::string> areaNames;
    std::ranges::transform(grid.gridElements,
                           std::inserter(areaNames, areaNames.end()),
                           &GridElement::area);

    write_joined(file,
                 areaNames
                   | std::views::transform([](const std::string& name)
                                           { return name + "_RHSValue"; }),
                 ",");
    file << ",cost,";
    write_joined(file,
                 areaNames
                   | std::views::transform([](const std::string& name)
                                           { return name + "_dualValue"; }),
                 ",");
    file << '\n';

    for (const auto& [pointScenarioWeek, gridPointRes]: values)
    {
        file << pointScenarioWeek.scenario << "," << pointScenarioWeek.week << ",";
        write_joined(file, pointScenarioWeek.rhsValues | std::views::values, ",");
        file << "," << gridPointRes.cost << ",";
        write_joined(file, gridPointRes.dual | std::views::values, ",");
        file << '\n';
    }
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
        const std::string verbosity = optionsParser.Verbosity();

        auto gridCollection = std::make_shared<GridCollection>(studyPath
                                                               / "user/water_values/grid.csv");

        const std::filesystem::path penaltiesConfigFilePath(studyPath
                                                            / "user/water_values/penalties.yaml");

        // PenaltiesConfigReader will check whether the file exists and return default values if
        // needed
        PenaltiesConfigReader pcr(penaltiesConfigFilePath);

        ReservoirManagement reservoirManagement(gridCollection->reservoirs.begin()->second,
                                                pcr.getPenaltyBottomRuleCurve(),
                                                pcr.getPenaltyUpperRuleCurve(),
                                                pcr.getPenaltyFinalLevel(),
                                                pcr.getForceFinalLevel(),
                                                pcr.getFinalLevel());

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
        Logger masterLogger = loggerFactory.get_logger();
        std::shared_ptr<FilteredLogger> logger = std::make_shared<FilteredLogger>(
          masterLogger,
          LogUtils::StrToLogLevel(verbosity));

        auto startProblemGeneration = std::chrono::system_clock::now();
        logger->display_message(
          "Generating problems (starting time: " + formatTime(startProblemGeneration) + ")");
        ProblemGenerationForWaterValueCalculation pbg(
          directories,
          logger,
          solverName,
          ProblemGenerationForWaterValueCalculation::getComputationModeFromGrid(
            ignoreOptimalTrajectory), // not used in grid_evaluator
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

        for (auto& grid: gridCollection->gridDefinitions | std::views::values)
        {
            auto startProblemUpdate = std::chrono::system_clock::now();
            logger->display_message(
              "Updating problems (starting time: " + formatTime(startProblemUpdate) + ")");
            auto problems = pbg.updateProblems(grid);

            auto endProblemUpdate = std::chrono::system_clock::now();
            logger->display_message("Updated problems (end time: " + formatTime(endProblemUpdate)
                                    + ")");

            std::chrono::duration<double> elapsed_update_seconds = endProblemUpdate
                                                                   - startProblemUpdate;
            logger->display_message("Elapsed time for problem update: "
                                    + formatDuration(elapsed_update_seconds));

            auto res = GridEvaluator(logger,
                                     problems,
                                     grid,
                                     solverName,
                                     directories.simulation_dir,
                                     nbThreads)
                         .ComputeCostsAndDuals();
            std::string fileName = "gridPointsValues_" + std::to_string(grid.gridID) + ".csv";
            saveCostsAndDuals(directories.simulation_dir / fileName, grid, res, logger);
            logger->display_message("Saved costs and duals to file");
        }

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
