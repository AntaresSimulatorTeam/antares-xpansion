#pragma once
#include <any>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#include "yaml-cpp/yaml.h"

class PenaltiesConfigReader
{
public:
    PenaltiesConfigReader(const std::filesystem::path& pathToYamlConfigFile = "");

    // these getters are still useful to enforce types
    double getPenaltyBottomRuleCurve()
    {
        return std::get<double>(penalties[penaltyBottomRuleCurveKey].getValue());
    }

    double getPenaltyUpperRuleCurve()
    {
        return std::get<double>(penalties[penaltyUpperRuleCurveKey].getValue());
    }

    double getPenaltyFinalLevel()
    {
        return std::get<double>(penalties[penaltyFinalLevelKey].getValue());
    }

    bool getForceFinalLevel()
    {
        return std::get<bool>(penalties[forceFinalLevelKey].getValue());
    }

    std::optional<double> getFinalLevel()
    {
        return std::get<std::optional<double>>(penalties[finalLevelKey].getValue());
    }

    bool getOverflow()
    {
        return std::get<bool>(penalties[overflowKey].getValue());
    }

private:
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
        Penalty(const std::string& key = "",
                const PenaltyValueType& defaultValue = 0.,
                bool isOptional = false);
        void updateValue(const YAML::Node& config);

        PenaltyValueType getValue()
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
    inline static const std::string overflowKey = "overflow";

    std::map<std::string, Penalty> penalties;

    std::string pathToYamlConfigFile;
    YAML::Node config;
};
