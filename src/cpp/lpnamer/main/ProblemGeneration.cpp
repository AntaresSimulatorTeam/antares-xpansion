
#include "include/ProblemGeneration.h"

#include <antares/api/SimulationResults.h>
#include <antares/api/solver.h>
#include <ittnotify.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <execution>
#include <filesystem>
#include <iostream>
#include <utility>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "FileProblemsProviderAdapter.h"
#include "GeneralDataReader.h"
#include "LauncherHelpers.h"
#include "LinkProblemsGenerator.h"
#include "LogUtils.h"
#include "LpFilesExtractor.h"
#include "MPSFileWriter.h"
#include "MasterGeneration.h"
#include "MasterProblemBuilder.h"
#include "MpsTxtWriter.h"
#include "ProblemGenerationLogger.h"
#include "ProblemVariablesFileAdapter.h"
#include "ProblemVariablesFromProblemAdapter.h"
#include "ProblemVariablesZipAdapter.h"
#include "StringManip.h"
#include "Timer.h"
#include "Version.h"
#include "WeightsFileReader.h"
#include "WeightsFileWriter.h"
#include "XpansionProblemsFromAntaresProvider.h"
#include "ZipProblemsProviderAdapter.h"
#include "config.h"
#include "include/memory.h"

static const std::string LP_DIRNAME = "lp";

namespace boost::serialization {
template <class Archive>
void serialize(Archive& ar, Antares::Solver::ConstantDataFromAntares& data, const unsigned int version) {
        ar& data.VariablesCount;
        ar& data.ConstraintesCount;
        ar& data.CoeffCount;
        ar& data.VariablesType;
        ar& data.Mdeb;
        ar& data.NotNullTermCount;
        ar& data.ColumnIndexes;
        ar& data.ConstraintsMatrixCoeff;
        ar& data.VariablesMeaning;
        ar& data.ConstraintsMeaning;
}

template <class Archive>
void serialize(Archive& ar, Antares::Solver::WeeklyProblemId& data, const unsigned int version) {
  ar& data.year;
  ar& data.week;
}


template <class Archive>
void serialize(Archive& ar, Antares::Solver::WeeklyDataFromAntares& data, const unsigned int version) {
  ar& data.Direction;
  ar& data.Xmax;
  ar& data.Xmin;
  ar& data.LinearCost;
  ar& data.RHS;
  ar& data.name;
  ar& data.variables;
  ar& data.constraints;
}

template <class Archive>
void serialize(Archive& ar, Antares::Solver::LpsFromAntares& data, const unsigned int version) {
        ar& data.constantProblemData;
        ar& data.weeklyProblems;
}
}  // namespace boost::serialization

namespace {
std::uintmax_t disk_usage_percent(const std::filesystem::space_info& si,
                                  bool is_privileged = false) noexcept
{
  if (constexpr std::uintmax_t X(-1);
      si.capacity == 0 || si.free == 0 || si.available == 0 ||
      si.capacity == X || si.free == X || si.available == X
  )
    return 100;

  std::uintmax_t unused_space = si.free, capacity = si.capacity;
  if (!is_privileged)
  {
    const std::uintmax_t privileged_only_space = si.free - si.available;
    unused_space -= privileged_only_space;
    capacity -= privileged_only_space;
  }
  const std::uintmax_t used_space{capacity - unused_space};
  return 100 * used_space / capacity;
}

void print_disk_space_info(auto const& dirs, int width = 14)
{
  (std::cout << std::left).imbue(std::locale("en_US.UTF-8"));

  for (const auto s : {"Capacity", "Free", "Available", "Use%", "Dir"})
    std::cout << "│ " << std::setw(width) << s << ' ';

  for (std::cout << '\n'; auto const& dir : dirs)
  {
    std::error_code ec;
    const std::filesystem::space_info si = std::filesystem::space(dir, ec);
    for (auto x : {si.capacity, si.free, si.available, disk_usage_percent(si)})
      std::cout << "│ " << std::setw(width) << static_cast<std::intmax_t>(x / 1024.f /1024.f) << ' ';
    std::cout << "│ " << dir << '\n';
  }
}

struct self_mem {
  long double vm_usage = 0.0;
  long resident_set = 0.0;
};
self_mem process_mem_usage()
{
  self_mem mem;

  // the two fields we want
  unsigned long vsize;
  long rss;
  {
    std::string ignore;
    std::ifstream ifs("/proc/self/stat", std::ios_base::in);
    ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
        >> ignore >> ignore >> vsize >> rss;
  }

  long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
  return {vsize / 1024.0l / 1024.l, rss * page_size_kb};
}
static std::string study_dir = "";

namespace fs = std::filesystem;
std::uintmax_t calculate_directory_size(const fs::path& directory) {
  std::uintmax_t total_size = 0;
  std::error_code ec;

  for (const auto& entry : fs::recursive_directory_iterator(directory, ec)) {
    if (ec) {
      std::cerr << "Error accessing " << entry.path() << ": " << ec.message() << "\n";
      continue;
    }
    if (fs::is_regular_file(entry.status())) {
      total_size += fs::file_size(entry.path(), ec);
      if (ec) {
        std::cerr << "Error getting size of " << entry.path() << ": " << ec.message() << "\n";
      }
    }
  }

  return total_size;
}

void memory() {
  auto [dispo, total] = Memory::MemoryUsageGo();
  auto [vm, rss] = process_mem_usage();
  std::cout << "====================================\n";
  std::cout << "Memory usage: " << dispo << "/" << total << "\n";
  std::cout << "VM: " << vm << "Mb; RSS: " << rss << "\n";
  std::cout << "------------------------------------\n";
  using namespace std::string_literals;
  const auto dirs = {"/scratch"s, "/tmp"s};
  print_disk_space_info(dirs);
  std::cout << "------------------------------------\n";
  std::uintmax_t dir_size = calculate_directory_size(study_dir);
  std::cout << "Total size of directory " << study_dir << " is " << dir_size / 1024.f / 1024.f << " Mo." << "\n";
  std::cout << "====================================" << "\n";
}
}

void CreateDirectories(const std::filesystem::path& output_path) {
  if (!std::filesystem::exists(output_path)) {
    std::filesystem::create_directories(output_path);
  }
  auto lp_path = output_path / LP_DIRNAME;
  if (!std::filesystem::exists(lp_path)) {
    std::filesystem::create_directories(lp_path);
  }
}

ProblemGeneration::ProblemGeneration(ProblemGenerationOptions& options)
    : options_(options) {
  if (!options_.StudyPath().empty()) {
    mode_ = SimulationInputMode::ANTARES_API;
  } else if (!options_.XpansionOutputDir().empty()) {
    mode_ = SimulationInputMode::FILE;
  } else if (!options_.ArchivePath().empty()) {
    mode_ = SimulationInputMode::ARCHIVE;
  }
}

std::filesystem::path ProblemGeneration::performAntaresSimulation() {
//#ifdef SAVE
//  auto results = Antares::API::PerformSimulation(options_.StudyPath());
//  //Add parallel
//  //Handle errors
//  if (results.error) {
//    std::cerr << "Error: " << results.error->reason << "\n";
//    exit(1);
//  }
//
//  lps_ = std::move(results.antares_problems);
//  std::ofstream ofs("lps.txt");
//  boost::archive::text_oarchive oa(ofs);
//  oa << lps_;
//  //TODO save simulation path
//  return {results.simulationPath};
//#else
//  std::ifstream ifs("lps.txt");
//  boost::archive::text_iarchive ia(ifs);
//  ia >> lps_;
//  return "/home/marechaljas/Téléchargements/study_1_integer/output/20240715-1416eco";
//#endif
{
  std::cout << "Memory usage before simulation:\n ";
  memory();
}
  auto results = Antares::API::PerformSimulation(options_.StudyPath());

  {
    std::cout << "Memory usage after simulation:\n";
    memory();
  }  //  //Add parallel
  //  //Handle errors
  if (results.error) {
    std::cerr << "Error: " << results.error->reason << "\n";
    exit(1);
  }
  lps_ = std::move(results.antares_problems);
  return {results.simulationPath};
}

std::filesystem::path ProblemGeneration::updateProblems() {
  using namespace std::string_literals;
  std::filesystem::path xpansion_output_dir;
  const auto archive_path = options_.ArchivePath();

  if (mode_ == SimulationInputMode::ARCHIVE) {
    xpansion_output_dir =
        options_.deduceXpansionDirIfEmpty(xpansion_output_dir, archive_path);
    study_dir = xpansion_output_dir.parent_path();

  }

  if (mode_ == SimulationInputMode::ANTARES_API) {
    simulation_dir_ = performAntaresSimulation();
    __itt_resume();
  }

  if (mode_ == SimulationInputMode::FILE) {
    simulation_dir_ = options_.XpansionOutputDir();  // Legacy naming.
    // options_.XpansionOutputDir() point in fact to a simulation output from
    // antares
    study_dir = simulation_dir_;
  }

  if (mode_ == SimulationInputMode::ANTARES_API || mode_ == SimulationInputMode::FILE) {
    xpansion_output_dir = simulation_dir_;
    study_dir = simulation_dir_;
  }


  const auto log_file_path =
      xpansion_output_dir / "lp"s / "ProblemGenerationLog.txt"s;

  CreateDirectories(xpansion_output_dir);  // Ca ou -Xpansion ?
  auto logger = ProblemGenerationLog::BuildLogger(log_file_path, std::cout,
                                                  "Problem Generation"s);

  auto master_formulation = options_.MasterFormulation();
  auto additionalConstraintFilename_l =
      options_.AdditionalConstraintsFilename();
  auto weights_file = options_.WeightsFile();
  auto unnamed_problems = options_.UnnamedProblems();

  RunProblemGeneration(xpansion_output_dir, master_formulation,
                       additionalConstraintFilename_l, archive_path, logger,
                       log_file_path, weights_file, unnamed_problems);
  return xpansion_output_dir;
}

std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path);
void ProblemGeneration::ProcessWeights(
    const std::filesystem::path& xpansion_output_dir,
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& weights_file,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  const auto settings_dir = xpansion_output_dir / ".." / ".." / "settings";
  const auto general_data_file = settings_dir / "generaldata.ini";
  auto genera_data_reader = GeneralDataIniReader(general_data_file, logger);
  auto active_years = genera_data_reader.GetActiveYears();
  WeightsFileReader weights_file_reader(weights_file, active_years.size(),
                                        logger);
  weights_file_reader.CheckWeightsFile();
  auto weights_vector = weights_file_reader.WeightsList();
  auto yearly_weight_writer = YearlyWeightsWriter(
      xpansion_output_dir, antares_archive_path, weights_vector,
      weights_file.filename(), active_years);
  yearly_weight_writer.CreateWeightFile();
}

void ProblemGeneration::ExtractUtilsFiles(
    const std::filesystem::path& antares_archive_path,
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger) {
  auto utils_files_extractor =
      LpFilesExtractor(antares_archive_path, xpansion_output_dir,
                       std::move(logger), mode_, simulation_dir_);
  utils_files_extractor.ExtractFiles();
}

std::vector<ActiveLink> getLinks(
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger) {
  ActiveLinksBuilder linkBuilder =
      get_link_builders(xpansion_output_dir, logger);
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
    const ProblemGenerationLog::ProblemGenerationLoggerSharedPointer& logger) {
  if ((master_formulation != "relaxed") && (master_formulation != "integer")) {
    (*logger)(LogUtils::LOGLEVEL::FATAL)
        << LOGLOCATION
        << "Invalid formulation argument : argument must be "
           "\"integer\" or \"relaxed\""
        << "\n";
    exit(1);
  }
}

std::vector<std::shared_ptr<Problem>> ProblemGeneration::getXpansionProblems(
    SolverLogManager& solver_log_manager, const std::string& solver_name,
    const std::vector<ProblemData>& mpsList, std::filesystem::path& lpDir_,
    std::shared_ptr<ArchiveReader>& reader, bool with_archive = true,
    const Antares::Solver::LpsFromAntares& lps = {}) {
  std::vector<std::string> problem_names;
  std::transform(mpsList.begin(), mpsList.end(),
                 std::back_inserter(problem_names),
                 [](ProblemData const& data) { return data._problem_mps; });
  switch (mode_) {
    case SimulationInputMode::FILE: {
      auto adapter =
          std::make_unique<FileProblemsProviderAdapter>(lpDir_, problem_names);
      return adapter->provideProblems(solver_name, solver_log_manager);
    }
    case SimulationInputMode::ARCHIVE: {
      auto adapter = std::make_unique<ZipProblemsProviderAdapter>(
          lpDir_, reader, problem_names);
      return adapter->provideProblems(solver_name, solver_log_manager);
    }
    case SimulationInputMode::ANTARES_API: {
      auto adapter = std::make_unique<XpansionProblemsFromAntaresProvider>(lps);
      return adapter->provideProblems(solver_name, solver_log_manager);
    }
    default:
      // TODO : log
      return {};
  }
}


void ProblemGeneration::RunProblemGeneration(
    const std::filesystem::path& xpansion_output_dir,
    const std::string& master_formulation,
    const std::string& additionalConstraintFilename_l,
    const std::filesystem::path& antares_archive_path,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger,
    const std::filesystem::path& log_file_path,
    const std::filesystem::path& weights_file, bool unnamed_problems) {
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Launching Problem Generation" << "\n";
  memory();
  validateMasterFormulation(master_formulation, logger);
  std::string solver_name = "CBC";  // TODO Use solver selected by user

  SolverLoader::GetAvailableSolvers(logger);  // Dirty fix to populate static
                                              // value outside multi thread code
  Timer problem_generation_timer;
  if (!weights_file.empty()) {
    ProcessWeights(xpansion_output_dir, antares_archive_path, weights_file,
                   logger);
  }
  //Print weight ok
  std::cout << "Weight ok" << "\n";
  memory();
  ExtractUtilsFiles(antares_archive_path, xpansion_output_dir, logger);

  std::vector<ActiveLink> links = getLinks(xpansion_output_dir, logger);
  std::cout << "Links ok" << "\n";
  memory();
  AdditionalConstraints additionalConstraints(logger);
  if (!additionalConstraintFilename_l.empty()) {
    additionalConstraints =
        AdditionalConstraints(additionalConstraintFilename_l, logger);
  }
  std::cout << "Additional constraints ok" << "\n";

  memory();
  auto lpDir_ = xpansion_output_dir / "lp";
  Version antares_version(ANTARES_VERSION);
  // TODO update the version of simulator that come with named mps
  Version first_version_without_variables_files("8.7");
  auto rename_problems =
      unnamed_problems ||
      antares_version < first_version_without_variables_files;
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "rename problems: " << std::boolalpha << rename_problems << "\n";



  memory();
  (*logger)(LogUtils::LOGLEVEL::INFO) << "File mapping" << "\n";
  auto files_mapper = FilesMapper(antares_archive_path, xpansion_output_dir);
  memory();
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Reading mps" << "\n";
  auto mpsList = files_mapper.MpsAndVariablesFilesVect();
  auto [dispo, total] = Memory::MemoryUsageGo();
  std::cout << "Memory usage: "
            << dispo << "/" << total << "\n";
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Reading mps done" << "\n";
  auto solver_log_manager = SolverLogManager(log_file_path);
  Couplings couplings;
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Reading couplings" << "\n";
  LinkProblemsGenerator linkProblemsGenerator(
      lpDir_, links, solver_name, logger, solver_log_manager, rename_problems);
  memory();
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Reading couplings done" << "\n";
  std::shared_ptr<ArchiveReader> reader =
      antares_archive_path.empty() ? std::make_shared<ArchiveReader>()
                                   : InstantiateZipReader(antares_archive_path);
  std::cout << "Archive opend\n";
  memory();
  /* Main stuff */

  std::vector<std::shared_ptr<Problem>> xpansion_problems =
      getXpansionProblems(solver_log_manager, solver_name, mpsList, lpDir_,
                          reader, !antares_archive_path.empty(), lps_);
  lps_.weeklyProblems.clear();
  lps_.constantProblemData = {};
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Problems read" << "\n";
  memory();
  std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
      problems_and_data;
  for (int i = 0; i < xpansion_problems.size(); ++i) {
    if (mode_ == SimulationInputMode::ANTARES_API) {
      ProblemData data{xpansion_problems.at(i)->_name, {}};
      problems_and_data.emplace_back(xpansion_problems.at(i), data);
    } else {
      xpansion_problems.at(i)->_name = mpsList.at(i)._problem_mps;
      problems_and_data.emplace_back(xpansion_problems.at(i), mpsList.at(i));
    }
  }
  (*logger)(LogUtils::LOGLEVEL::INFO) << "Start problem generation" << "\n";
  memory();
  auto mps_file_writer = std::make_shared<MPSFileWriter>(lpDir_);
  std::for_each(
      std::execution::par, problems_and_data.begin(), problems_and_data.end(),
      [&](const auto& problem_and_data) {
        const auto& [problem, data] = problem_and_data;
        std::cout << "Start " << data._problem_mps << "\n";
        std::cout << "Memory usage subproblem "<<
            data._problem_mps << " mcyear " << problem->McYear() << " :\n";
        memory();

        std::shared_ptr<IProblemVariablesProviderPort> variables_provider;
        switch (mode_) {
          case SimulationInputMode::FILE:
            variables_provider = std::make_shared<ProblemVariablesFileAdapter>(
                data, links, logger, lpDir_);
            break;
          case SimulationInputMode::ARCHIVE:
            if (rename_problems) {
              variables_provider = std::make_shared<ProblemVariablesZipAdapter>(
                  reader, data, links, logger);
            } else {
              variables_provider =
                  std::make_shared<ProblemVariablesFromProblemAdapter>(
                      problem, links, logger);
            }
            break;
          case SimulationInputMode::ANTARES_API:
            variables_provider =
                std::make_shared<ProblemVariablesFromProblemAdapter>(
                    problem, links, logger);
            break;
          default:
            (*logger)(LogUtils::LOGLEVEL::ERR) << "Undefined mode";
            break;
        }
        linkProblemsGenerator.treat(data._problem_mps, couplings, problem.get(),
                                    variables_provider.get(),
                                    mps_file_writer.get());
        std::cout << "End " << data._problem_mps << "\n";

      });

  if (mode_ == SimulationInputMode::ARCHIVE) {
    reader->Close();
    reader->Delete();
  }
  MasterGeneration master_generation(
      xpansion_output_dir, links, additionalConstraints, couplings,
      master_formulation, solver_name, logger, solver_log_manager);
  (*logger)(LogUtils::LOGLEVEL::INFO)
      << "Problem Generation ran in: "
      << format_time_str(problem_generation_timer.elapsed()) << "\n";
}
std::shared_ptr<ArchiveReader> InstantiateZipReader(
    const std::filesystem::path& antares_archive_path) {
  auto reader = std::make_shared<ArchiveReader>(antares_archive_path);
  reader->Open();
  return reader;
}
