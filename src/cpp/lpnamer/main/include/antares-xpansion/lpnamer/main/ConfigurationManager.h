#pragma once
#include <filesystem>
#include <optional>

#include "antares-xpansion/lpnamer/model/SimulationInputMode.h"

class ProblemGenerationOptions;

class ConfigurationManager
{
public:
    struct ConfigDirectories
    {
        std::filesystem::path xpansion_output_dir;
        std::filesystem::path study_dir;
        std::filesystem::path simulation_dir;
        std::filesystem::path archive_path;
    };

    explicit ConfigurationManager(ProblemGenerationOptions& options);
    auto Directories() -> ConfigDirectories;
    auto Mode() -> SimulationInputMode;
    std::filesystem::path generateOutputName(const std::filesystem::path& study);
    const ProblemGenerationOptions& options_;

private:
    std::optional<SimulationInputMode> input_mode_;
};
