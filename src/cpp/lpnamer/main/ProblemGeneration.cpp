#include "antares-xpansion/lpnamer/main/ProblemGeneration.h"

#include <execution>
#include <iostream>
#include <tbb/tbb.h>
#include <utility>

#include <antares/api/solver.h>

#include "Version.h"
#include "antares-xpansion/helpers/Timer.h"
#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"
#include "antares-xpansion/lpnamer/input_reader/LpFilesExtractor.h"
#include "antares-xpansion/lpnamer/input_reader/SettingsReader.h"
#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include "antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h"
#include "antares-xpansion/lpnamer/problem_modifier/FileProblemsProviderAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/FileWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFileAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/ProblemVariablesZipAdapter.h"
#include "antares-xpansion/lpnamer/problem_modifier/WeightFileProcessor.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "antares-xpansion/lpnamer/problem_modifier/ZipProblemsProviderAdapter.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"
#include "antares-xpansion/xpansion_interfaces/StringManip.h"
#include "config.h"
#include "malloc.h"

static const std::string LP_DIRNAME = "lp";

void CreateDirectories(const std::filesystem::path& output_path)
{
    if (!std::filesystem::exists(output_path))
    {
        std::filesystem::create_directories(output_path);
    }
    auto lp_path = output_path / LP_DIRNAME;
    if (!std::filesystem::exists(lp_path))
    {
        std::filesystem::create_directories(lp_path);
    }
}

ProblemGeneration::ProblemGeneration(ProblemGenerationOptions& options):
    options_(options),
    configuration_manager_{options}
{
    mode_ = configuration_manager_.Mode();
}

namespace
{
bool islower(std::string_view str)
{
    return std::ranges::all_of(str, [](char c) { return std::islower(c); });
}
} // namespace

static std::string solverXpansionToSimulator(const SolverConfig& in)
{
    // in could be Cbc or CBC depending on whether it is defined or not in the
    // settings file
    // Use lowerCase in any case to be robust to these subtleties
    assert(islower(in.Name()));
    if (in.Name() == "xpress")
    {
        return "xpress";
    }
    if (in.Name() == "cbc" || in.Name() == "coin")
    {
        return "coin";
    }
    throw std::invalid_argument("Invalid solver");
}

void ProblemGeneration::performAntaresSimulation(const std::filesystem::path& output)
{
    Antares::Solver::Optimization::OptimizationOptions optOptions;

    auto solver_name = solverXpansionToSimulator(solver_config_);
    optOptions.firstOptimOptions.solverName = solver_name;
    optOptions.firstOptimOptions.solverUsesBasis = true;
    optOptions.firstOptimOptions.solverExportsBasis = true;

    optOptions.secondOptimOptions.solverName = solver_name;
    optOptions.secondOptimOptions.solverUsesBasis = true;
    optOptions.secondOptimOptions.solverExportsBasis = false;

    if (solver_name == SolverConfig("xpress"))
    {
        optOptions.firstOptimOptions.solverParameters = "PRESOLVE 1";
        optOptions.secondOptimOptions.solverParameters = "PRESOLVE 1";
    }
    auto results = Antares::API::PerformSimulation(options_.StudyPath(), output, optOptions);

    /**
     * Antares simulator allocate a lot of memory
     * Even if there is no memory leak not all freed memory become available.
     * Allocator or OS may cache some memory to reuse it
     * With malloc_trim(0) we free all memory that is not used anymore to be reclaimed by the
     *program It is nescasssry to avoid allocating Xpansion memory on top of the unavailable memory
     *from simulator
     **/
#ifndef _WIN32
    malloc_trim(0);
#endif

    // Handle errors
    if (results.error)
    {
        throw LogUtils::XpansionError<std::runtime_error>("Antares simulation failed:\n\t"
                                                            + results.error->reason,
                                                          LOGLOCATION);
    }

    lps_ = std::move(results.antares_problems);
}

std::filesystem::path ProblemGeneration::updateProblems()
{
    using namespace std::string_literals;
    directories_ = configuration_manager_.Directories();

    const auto log_file_path = directories_.xpansion_output_dir / "lp"s
                               / "ProblemGenerationLog.txt"s;

    CreateDirectories(directories_.xpansion_output_dir);
    auto logger = ProblemGenerationLog::BuildLogger(log_file_path,
                                                    std::cout,
                                                    "Problem Generation"s);

    set_solver(directories_.study_dir, logger.get());

    if (mode_ == SimulationInputMode::ANTARES_API)
    {
        performAntaresSimulation(directories_.simulation_dir);
    }

    auto master_formulation = options_.MasterFormulation();
    auto additionalConstraintFilename_l = options_.AdditionalConstraintsFilename();
    auto weights_file = options_.WeightsFile();
    auto unnamed_problems = options_.UnnamedProblems();

    RunProblemGeneration(directories_.xpansion_output_dir,
                         master_formulation,
                         additionalConstraintFilename_l,
                         directories_.archive_path,
                         logger,
                         log_file_path,
                         weights_file,
                         unnamed_problems);
    return directories_.xpansion_output_dir;
}

std::shared_ptr<ArchiveReader> InstantiateZipReader(
  const std::filesystem::path& antares_archive_path);

void ProblemGeneration::ExtractUtilsFiles(
  const std::filesystem::path& antares_archive_path,
  const std::filesystem::path& xpansion_output_dir,
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger)
{
    auto utils_files_extractor = LpFilesExtractor(antares_archive_path,
                                                  xpansion_output_dir,
                                                  std::move(logger),
                                                  mode_.value(),
                                                  directories_.simulation_dir);
    utils_files_extractor.ExtractFiles();
}

std::vector<ActiveLink> getLinks(
  const std::filesystem::path& xpansion_output_dir,
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger>& logger)
{
    ActiveLinksBuilder linkBuilder = get_link_builders(xpansion_output_dir, logger);
    std::vector<ActiveLink> links = linkBuilder.getLinks();
    return links;
}

/**
 * TODO Move earlier in the process
 * @param master_formulation
 * @param logger
 */
void validateMasterFormulation(
  const std::string& master_formulation,
  const std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger)
{
    if ((master_formulation != "relaxed") && (master_formulation != "integer"))
    {
        (*logger)(LogUtils::LOGLEVEL::FATAL) << LOGLOCATION
                                             << "Invalid formulation argument : argument must be "
                                                "\"integer\" or \"relaxed\""
                                             << std::endl;
        exit(1);
    }
}

/**
 *
 * @param solver_log_manager
 * @param solver_name
 * @param mpsList
 * @param lpDir_
 * @param reader shared pointer to the archive reader to share with
 * ZipProblemsProviderAdapter
 * @param with_archive
 * @param lps data from antares. Passed by reference to prevent heavy copy
 * @return
 */
std::vector<std::shared_ptr<Problem>> ProblemGeneration::getXpansionProblems(
  SolverLogManager& solver_log_manager,
  const std::vector<ProblemData>& mpsList,
  std::filesystem::path& lpDir_,
  std::shared_ptr<ArchiveReader> reader,
  const Antares::Solver::LpsFromAntares& lps = {})
{
    std::vector<std::string> problem_names;
    std::transform(mpsList.begin(),
                   mpsList.end(),
                   std::back_inserter(problem_names),
                   [](const ProblemData& data) { return data._problem_filename; });
    switch (mode_.value())
    {
    case SimulationInputMode::FILE:
    {
        FileProblemsProviderAdapter adapter(lpDir_, problem_names);
        return adapter.provideProblems(solver_config_.Name(), solver_log_manager);
    }
    case SimulationInputMode::ARCHIVE:
    {
        ZipProblemsProviderAdapter adapter(lpDir_, std::move(reader), problem_names);
        return adapter.provideProblems(solver_config_.Name(), solver_log_manager);
    }
    case SimulationInputMode::ANTARES_API:
    {
        XpansionProblemsFromAntaresProvider adapter(lps);
        return adapter.provideProblems(solver_config_.Name(), solver_log_manager);
    }
    default:
        throw LogUtils::XpansionError<std::runtime_error>("Unhandled simulation mode", LOGLOCATION);
    }
}

void ProblemGeneration::RunProblemGeneration(
  const std::filesystem::path& xpansion_output_dir,
  const std::string& master_formulation,
  const std::string& additionalConstraintFilename_l,
  const std::filesystem::path& antares_archive_path,
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger,
  const std::filesystem::path& log_file_path,
  const std::filesystem::path& weights_file,
  bool unnamed_problems)
{
    (*logger)(LogUtils::LOGLEVEL::INFO) << "Launching Problem Generation" << std::endl;
    validateMasterFormulation(master_formulation, logger);

    SolverLoader::GetAvailableSolvers(logger); // Dirty fix to populate static
                                               // value outside multi thread code
    Timer problem_generation_timer;

    ExtractUtilsFiles(antares_archive_path, xpansion_output_dir, logger);

    std::vector<ActiveLink> links = getLinks(xpansion_output_dir, logger);

    AdditionalConstraints additionalConstraints(logger);
    if (!additionalConstraintFilename_l.empty())
    {
        additionalConstraints = AdditionalConstraints(additionalConstraintFilename_l, logger);
    }

    auto lpDir_ = xpansion_output_dir / "lp";
    Version antares_version(ANTARES_VERSION);
    // TODO update the version of simulator that come with named mps
    Version first_version_without_variables_files("8.7");
    auto rename_problems = unnamed_problems
                           || antares_version < first_version_without_variables_files;
    (*logger)(LogUtils::LOGLEVEL::INFO)
      << "rename problems: " << std::boolalpha << rename_problems << std::endl;

    auto files_mapper = FilesMapper(antares_archive_path, xpansion_output_dir);
    auto mpsList = files_mapper.MpsAndVariablesFilesVect();

    auto solver_log_manager = SolverLogManager(log_file_path);
    Couplings couplings;
    LinkProblemsGenerator linkProblemsGenerator(lpDir_,
                                                links,
                                                solver_config_,
                                                logger,
                                                solver_log_manager,
                                                rename_problems);
    std::shared_ptr<ArchiveReader> reader = mode_ == SimulationInputMode::ARCHIVE
                                              ? InstantiateZipReader(antares_archive_path)
                                              : std::make_shared<ArchiveReader>();

    /* Main stuff */
    if (mode_ == SimulationInputMode::FILE or mode_ == SimulationInputMode::ARCHIVE)
    {
        (*logger)(LogUtils::LOGLEVEL::INFO) << "Collecting problems...";
        std::vector<std::shared_ptr<Problem>> xpansion_problems = getXpansionProblems(
          solver_log_manager,
          mpsList,
          lpDir_,
          reader,
          lps_);
        (*logger)(LogUtils::LOGLEVEL::INFO) << " Done.\n";

        std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>> problems_and_data;
        for (int i = 0; i < xpansion_problems.size(); ++i)
        {
            xpansion_problems.at(i)->_name = mpsList.at(i)._problem_filename;
            problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
        }
        auto mps_file_writer = std::make_shared<FileWriter>(configuration_manager_.Format());
        std::for_each(
          std::execution::par,
          problems_and_data.begin(),
          problems_and_data.end(),
          [&](const auto& problem_and_data)
          {
              const auto& [problem, data] = problem_and_data;
              std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
              switch (mode_.value())
              {
              case SimulationInputMode::FILE:
                  variables_provider = std::make_shared<ProblemVariablesFileAdapter>(data,
                                                                                     links,
                                                                                     logger,
                                                                                     lpDir_);
                  break;
              case SimulationInputMode::ARCHIVE:
                  if (rename_problems)
                  {
                      variables_provider = std::make_shared<ProblemVariablesZipAdapter>(reader,
                                                                                        data,
                                                                                        links,
                                                                                        logger);
                  }
                  else
                  {
                      variables_provider = std::make_shared<ProblemVariablesFromProblemAdapter>(
                        problem,
                        links,
                        logger);
                  }
                  break;
              default:
                  (*logger)(LogUtils::LOGLEVEL::ERR) << "Undefined mode";
                  break;
              }
              linkProblemsGenerator.treat(data._problem_filename,
                                          couplings,
                                          problem.get(),
                                          variables_provider.get(),
                                          mps_file_writer.get());
          });
        WeightFileProcessor weights_file_processor;
        weights_file_processor.ProcessWeights(problems_and_data,
                                              xpansion_output_dir,
                                              weights_file,
                                              solver_config_.Name(),
                                              logger);
    }
    else // API
    {
        auto mps_file_writer = std::make_shared<FileWriter>(configuration_manager_.Format());

        // vector of pair for parallelization
        // ref to WeeklyDataFromAntares to avoid copies
        std::vector<
          std::pair<Antares::Solver::WeeklyProblemId, Antares::Solver::WeeklyDataFromAntares&>>
          weekly_data;
        std::ranges::for_each(lps_.weeklyProblems,
                              [&weekly_data](auto& pair)
                              { weekly_data.emplace_back(pair.first, pair.second); });

        std::vector<std::pair<int, ProblemData>> year_and_data;
        year_and_data.reserve(lps_.weeklyProblems.size());
        std::mutex mutex;
        std::for_each(
          std::execution::par,
          weekly_data.begin(),
          weekly_data.end(),
          [&](const auto& weeklyDataByYearWeek)
          {
              auto&& [year_week, data] = weeklyDataByYearWeek;
              XpansionProblemsFromAntaresProvider adapter(lps_);
              auto problem = adapter.provideProblem(solver_config_.Name(),
                                                    solver_log_manager,
                                                    year_week);
              {
                  std::lock_guard guard(mutex);
                  lps_.weeklyProblems.erase(year_week); // Clear data to save memory
                  year_and_data.emplace_back(problem->mc_year, ProblemData{problem->_name, {}});
                  // Need to be done before treat because it will update problem name with the full
                  // path
              }
              std::shared_ptr<IProblemVariablesProviderPort>
                variables_provider = std::make_shared<ProblemVariablesFromProblemAdapter>(problem,
                                                                                          links,
                                                                                          logger);

              linkProblemsGenerator.treat(problem->_name,
                                          couplings,
                                          problem.get(),
                                          variables_provider.get(),
                                          mps_file_writer.get());
          });
        WeightFileProcessor weights_file_processor;
        weights_file_processor.ProcessWeights(year_and_data,
                                              xpansion_output_dir,
                                              weights_file,
                                              solver_config_.Name(),
                                              logger);
    }

    if (mode_ == SimulationInputMode::ARCHIVE)
    {
        reader->Close();
        reader->Delete();
    }
    FileWriter file_writer(configuration_manager_.Format());
    MasterGeneration master_generation(xpansion_output_dir,
                                       links,
                                       additionalConstraints,
                                       couplings,
                                       master_formulation,
                                       solver_config_.Name(),
                                       logger,
                                       solver_log_manager,
                                       file_writer);
    (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Problem Generation ran in: " << format_time_str(problem_generation_timer.elapsed())
      << std::endl;
}

void ProblemGeneration::set_solver(std::filesystem::path study_dir,
                                   ProblemGenerationLog::ProblemGenerationLogger* logger)
{
    SettingsReader settingsReader(study_dir / "user" / "expansion" / "settings.ini", logger);
    solver_config_ = settingsReader.Solver();
}

std::shared_ptr<ArchiveReader> InstantiateZipReader(
  const std::filesystem::path& antares_archive_path)
{
    auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
    reader->Open();
    return reader;
}
