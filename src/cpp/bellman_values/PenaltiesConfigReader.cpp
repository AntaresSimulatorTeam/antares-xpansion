#include "include/antares-xpansion/bellman_values/PenaltiesConfigReader.h"

/// @brief Constructor for a Penalty class holding a name (key), defaultValue, actual value, and
/// whether it is optional
/// @param key The key as found in a YAML file
/// @param defaultValue The default value if none is found in YAML, if a ~ is used, or if no file is
/// present.
/// @param isOptional A convenient boolean to handle std::optional<> type values
PenaltiesConfigReader::Penalty::Penalty(const std::string& key,
                                        const PenaltyValueType& defaultValue,
                                        bool isOptional):
    key(key),
    defaultValue(defaultValue),
    value(defaultValue),
    isOptional(isOptional)
{
}

/// @brief updates a penalty held value based on the content of a YAML config file and its default
/// value if needed
/// @param config a const reference to the YAML tree parsed from a file
void PenaltiesConfigReader::Penalty::updateValue(const YAML::Node& config)
{
    try
    {
        // if key exists and isn't empty
        if (config[key] && !config[key].IsNull())
        {
            if (isOptional)
            {
                if (std::holds_alternative<std::optional<double>>(defaultValue))
                {
                    value = std::optional<double>(config[key].as<double>());
                    return;
                }
            }
            else
            {
                if (std::holds_alternative<double>(defaultValue))
                {
                    value = config[key].as<double>();
                    return;
                }
                if (std::holds_alternative<bool>(defaultValue))
                {
                    value = config[key].as<bool>();
                    return;
                }
            }
        }
        else
        {
            // fall back on default value
            value = defaultValue;
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Error when parsing YAML penalties file: "
                                 + std::string(e.what()));
    }
}

/// @brief Constructor to determine all penalties-related parameters' values. Here is where one can
/// add new parameters to parse from a YAML file if needed (getters would need to be implemented as
/// well).
/// @param pathToYamlConfigFile A path to the YAML file
PenaltiesConfigReader::PenaltiesConfigReader(const std::filesystem::path& pathToYamlConfigFile)
{
    // instantiate all penalties with key (as in YAML), default value, and boolean for optional
    penalties.emplace(penaltyBottomRuleCurveKey, Penalty(penaltyBottomRuleCurveKey, 0.0));
    penalties.emplace(penaltyUpperRuleCurveKey, Penalty(penaltyUpperRuleCurveKey, 0.0));
    penalties.emplace(penaltyFinalLevelKey, Penalty(penaltyFinalLevelKey, 0.0));
    penalties.emplace(forceFinalLevelKey, Penalty(forceFinalLevelKey, false));
    penalties.emplace(finalLevelKey, Penalty(finalLevelKey, std::optional<double>{}, true));
    penalties.emplace(overflowKey, Penalty(overflowKey, true));
    if (std::filesystem::exists(pathToYamlConfigFile))
    {
        // read values
        config = YAML::LoadFile(pathToYamlConfigFile.string());

        for (auto& [key, penalty]: penalties)
        {
            penalty.updateValue(config);
        }
    }
    else
    {
        // falling back on default values
        std::cout << "No penalties.yaml file found, falling back on default values.\n";
    }
}
