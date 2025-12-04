#pragma once
#include <antares-xpansion/core/ProblemFormat.h>
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
    auto Directories() const -> ConfigDirectories;
    auto Mode() const -> SimulationInputMode;
    auto Format() const -> ProblemsFormat;
    std::filesystem::path generateOutputName(const std::filesystem::path& study) const;
    const ProblemGenerationOptions& options_;

private:
    mutable std::optional<SimulationInputMode> input_mode_;
    ProblemsFormat format_{ProblemsFormat::OPTIMIZED};
    mutable std::optional<ConfigDirectories> directories_;
};
