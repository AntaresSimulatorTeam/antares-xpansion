
#include "antares-xpansion/lpnamer/main/ConfigurationManager.h"

#include "antares-xpansion/lpnamer/main/ProblemGenerationOptions.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

ConfigurationManager::ConfigurationManager(ProblemGenerationOptions& options):
    options_{options},
    format_{options_.Format()}
{
    format_ = options_.Format();
}

auto ConfigurationManager::Directories() const -> ConfigDirectories
{
    if (directories_)
    {
        return *directories_;
    }
    Mode();
    std::filesystem::path xpansion_output_dir;
    std::filesystem::path simulation_dir_;
    const auto archive_path = options_.ArchivePath();
    std::filesystem::path study_dir;

    if (input_mode_ == SimulationInputMode::ARCHIVE)
    {
        xpansion_output_dir = options_.deduceXpansionDirIfEmpty(xpansion_output_dir, archive_path);
        study_dir = std::filesystem::absolute(archive_path).parent_path().parent_path();
        // Assume study/output/archive.zip
        // If doesn't work
        // study_dir = xpansion_output_dir.parent_path().parent_path(); //Assume
        // study/output/archive.zip
    }

    if (input_mode_ == SimulationInputMode::ANTARES_API)
    {
        study_dir = options_.StudyPath();
        simulation_dir_ = generateOutputName(study_dir);
    }

    if (input_mode_ == SimulationInputMode::FILE)
    {
        simulation_dir_ = options_.XpansionOutputDir(); // Legacy naming.
        // options_.XpansionOutputDir() point in fact to a simulation output from
        // antares
        study_dir = std::filesystem::absolute(simulation_dir_)
                      .parent_path()
                      .parent_path(); // Assume study/output/simulation
    }

    if (input_mode_ == SimulationInputMode::ANTARES_API || input_mode_ == SimulationInputMode::FILE)
    {
        xpansion_output_dir = simulation_dir_;
    }
    directories_ = ConfigDirectories{.xpansion_output_dir = xpansion_output_dir,
                                     .study_dir = study_dir,
                                     .simulation_dir = simulation_dir_,
                                     .archive_path = archive_path};
    return *directories_;
}

auto ConfigurationManager::Mode() const -> SimulationInputMode
{
    if (input_mode_)
    {
        return *input_mode_;
    }
    if (!options_.StudyPath().empty())
    {
        input_mode_ = SimulationInputMode::ANTARES_API;
        return *input_mode_;
    }
    if (!options_.XpansionOutputDir().empty())
    {
        input_mode_ = SimulationInputMode::FILE;
        return *input_mode_;
    }
    if (!options_.ArchivePath().empty())
    {
        input_mode_ = SimulationInputMode::ARCHIVE;
        return *input_mode_;
    }
    throw LogUtils::XpansionError<std::runtime_error>("SimulationInputMode is unknown",
                                                      LOGLOCATION);
}

auto ConfigurationManager::Format() const -> ProblemsFormat
{
    return format_;
}

namespace
{
std::string getCurrentTimestamp()
{
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for formatting
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to tm structure
    std::tm now_tm;
#ifdef _WIN32
    localtime_s(&now_tm, &now_time_t); // Windows-specific
#else
    localtime_r(&now_time_t, &now_tm); // POSIX-specific
#endif

    // Format the timestamp
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y%m%d-%H%Meco");
    return oss.str();
}
} // namespace

std::filesystem::path ConfigurationManager::generateOutputName(const std::filesystem::path& study)
{
    auto name = study / "output" / getCurrentTimestamp();
    if (std::filesystem::exists(name))
    {
        int counter = 1;
        std::filesystem::path new_name;
        do
        {
            new_name = name.concat("_" + std::to_string(counter));
            counter++;
        } while (std::filesystem::exists(new_name));
        return new_name;
    }
    return name;
}
