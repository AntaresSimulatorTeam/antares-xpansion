#pragma once
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "yaml-cpp/yaml.h"

/**
 * @brief An abstract class to parse configuration YAML files, like penalties or other parameters,
 * or user-defined settings
 *
 */
class ConfigReader
{
public:
    // so far, choices in the YAML file types are these
    typedef std::variant<bool, double, int, std::optional<double>, std::string> ElementValueType;

    /**
     * @brief A YAMLElement is a structure defining an element to be read in a YAML file, as a
     * string key, value read in file, default value to fall back on, and whether the element is
     * optional.
     *
     */
    class YAMLElement
    {
    private:
        ElementValueType defaultValue;
        ElementValueType value;
        std::string key;
        bool isOptional;

    public:
        YAMLElement(const std::string& key,
                    const ElementValueType& defaultValue,
                    bool isOptional = false):
            key(key),
            defaultValue(defaultValue),
            value(defaultValue),
            isOptional(isOptional)
        {
        }

        /// @brief updates a YAML element held value based on the content of a YAML config file and
        /// its default value if needed
        /// @param config a const reference to the YAML tree parsed from a file
        void updateValue(const YAML::Node& config);

        ElementValueType getValue() const
        {
            return value;
        }
    };

    /**
     * @brief Construct a new Config Reader object. The derived constructors must call
     * emplaceAllElements (to be overriden) and initializeAllElements (already concrete)
     *
     */
    ConfigReader() = default;
    virtual ~ConfigReader() = 0;

protected:
    template<typename T>
    T getValueFromKey(const std::string& key,
                      const std::map<std::string, ConfigReader::YAMLElement>& elements) const
    {
        auto it = elements.find(key);
        if (it != elements.end())
        {
            return std::get<T>(it->second.getValue());
        }
        // for good measure: raise an error if no value found
        throw std::runtime_error("Value was not found in YAML file " + pathToYamlConfigFile_
                                 + "for key: " + key);
    }

    /**
     * @brief A virtual function to be implemented in concrete classes, where YAML elements are
     * emplaced in elements_ using a string key and YAMLElement instance
     *
     */
    virtual void emplaceAllElements() = 0;

    /**
     * @brief A function to initialize all elements with their default value if the file is not
     * found
     *
     * @param pathToYamlConfigFile
     */
    void initializeAllElements(const std::filesystem::path& pathToYamlConfigFile);

    /**
     * @brief a map holding all expected elements from the YAML by their key, to be defined by
     * derived classes
     *
     */
    std::map<std::string, YAMLElement> elements_;

    /**
     * @brief the path to the YAML configuration file to be parsed
     *
     */
    std::string pathToYamlConfigFile_;

    /**
     * @brief a yaml-cpp node to be populated by all elements
     *
     */
    YAML::Node config_;
};
