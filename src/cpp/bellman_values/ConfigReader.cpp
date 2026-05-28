#include "include/antares-xpansion/bellman_values/ConfigReader.h"

ConfigReader::~ConfigReader()
{
}

void ConfigReader::YAMLElement::updateValue(const YAML::Node& config)
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
                if (std::holds_alternative<int>(defaultValue))
                {
                    value = config[key].as<int>();
                    return;
                }
                if (std::holds_alternative<bool>(defaultValue))
                {
                    value = config[key].as<bool>();
                    return;
                }
                if (std::holds_alternative<std::string>(defaultValue))
                {
                    value = config[key].as<std::string>();
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
        std::throw_with_nested(
          std::runtime_error("Error parsing YAML file for key: " + key + "(" + e.what() + ")"));
    }
}

void ConfigReader::initializeAllElements(const std::filesystem::path& pathToYamlConfigFile)
{
    if (std::filesystem::exists(pathToYamlConfigFile))
    {
        std::cout << "Reading values in " << pathToYamlConfigFile.string() << std::endl;
        // read values
        config_ = YAML::LoadFile(pathToYamlConfigFile.string());

        for (auto& [key, element]: elements_)
        {
            element.updateValue(config_);
            std::cout << "Parsing value for " << key << std::endl;
        }
    }
    else
    {
        // falling back on default values
        std::cout << "No yaml file found at the specified location ("
                       + pathToYamlConfigFile.string() + "), falling back on default values.\n";
    }
}
