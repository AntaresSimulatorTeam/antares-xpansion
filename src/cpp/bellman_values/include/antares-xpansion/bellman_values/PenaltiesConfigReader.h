#pragma once
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "yaml-cpp/yaml.h"

class PenaltiesConfigReader
{
public:
    PenaltiesConfigReader(const std::filesystem::path& pathToYamlConfigFile = "");

    // these getters are still useful to enforce types
    double getPenaltyBottomRuleCurve() const
    {
        return getValueFromKey<double>(penaltyBottomRuleCurveKey);
    }

    double getPenaltyUpperRuleCurve() const
    {
        return getValueFromKey<double>(penaltyUpperRuleCurveKey);
    }

    double getPenaltyFinalLevel() const
    {
        return getValueFromKey<double>(penaltyFinalLevelKey);
    }

    bool getForceFinalLevel() const
    {
        return getValueFromKey<bool>(forceFinalLevelKey);
    }

    std::optional<double> getFinalLevel() const
    {
        return getValueFromKey<std::optional<double>>(finalLevelKey);
    }

private:
    template<typename T>
    T getValueFromKey(const std::string& key) const
    {
        auto it = penalties.find(key);
        if (it != penalties.end())
        {
            return std::get<T>(it->second.getValue());
        }
        // for good measure: raise an error if no value found
        throw std::runtime_error("Value was not found in penalties for key: " + key);
    }

    // so far, choices in the YAML file types are these
    typedef std::variant<bool, double, std::optional<double>> PenaltyValueType;

    class Penalty
    {
    private:
        PenaltyValueType defaultValue;
        PenaltyValueType value;
        std::string key;
        bool isOptional;

    public:
        Penalty(const std::string& key,
                const PenaltyValueType& defaultValue,
                bool isOptional = false);
        void updateValue(const YAML::Node& config);

        PenaltyValueType getValue() const
        {
            return value;
        }
    };

    // keys from YAML file:
    inline static const std::string penaltyBottomRuleCurveKey = "penalty_bottom_rule_curve";
    inline static const std::string penaltyUpperRuleCurveKey = "penalty_upper_rule_curve";
    inline static const std::string penaltyFinalLevelKey = "penalty_final_level";
    inline static const std::string forceFinalLevelKey = "force_final_level";
    inline static const std::string finalLevelKey = "final_level";

    std::map<std::string, Penalty> penalties;

    std::string pathToYamlConfigFile;
    YAML::Node config;
};
