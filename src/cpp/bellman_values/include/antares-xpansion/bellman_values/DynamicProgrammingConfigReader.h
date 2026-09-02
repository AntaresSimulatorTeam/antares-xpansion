#pragma once
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "ConfigReader.h"
#include "yaml-cpp/yaml.h"

class DynamicProgrammingConfigReader: ConfigReader
{
public:
    DynamicProgrammingConfigReader(const std::filesystem::path& pathToYamlConfigFile = "")
    {
        emplaceAllElements(); // this method must be defined in the derived class
        initializeAllElements(pathToYamlConfigFile); // this method comes from the base class
        parsePenaltiesByArea();                      // specifically for penalties by area
    }

    // these getters are still useful to select an area, and to enforce types
    double getPenaltyBottomRuleCurveForArea(std::string area) const
    {
        if (penalties_.count(area) > 0)
        {
            return getValueFromKey<double>(penaltyBottomRuleCurveKey, penalties_.at(area));
        }
        return getValueFromKey<double>(penaltyBottomRuleCurveKey, defaultPenalty_);
    }

    double getPenaltyUpperRuleCurveForArea(std::string area) const
    {
        if (penalties_.count(area) > 0)
        {
            return getValueFromKey<double>(penaltyUpperRuleCurveKey, penalties_.at(area));
        }
        return getValueFromKey<double>(penaltyUpperRuleCurveKey, defaultPenalty_);
    }

    double getPenaltyFinalLevelForArea(std::string area) const
    {
        if (penalties_.count(area) > 0)
        {
            return getValueFromKey<double>(penaltyFinalLevelKey, penalties_.at(area));
        }
        return getValueFromKey<double>(penaltyFinalLevelKey, defaultPenalty_);
    }

    bool getForceFinalLevelForArea(std::string area) const
    {
        if (penalties_.count(area) > 0)
        {
            return getValueFromKey<bool>(forceFinalLevelKey, penalties_.at(area));
        }
        return getValueFromKey<bool>(forceFinalLevelKey, defaultPenalty_);
    }

    std::optional<double> getFinalLevelForArea(std::string area) const
    {
        if (penalties_.count(area) > 0)
        {
            return getValueFromKey<std::optional<double>>(finalLevelKey, penalties_.at(area));
        }
        return getValueFromKey<std::optional<double>>(finalLevelKey, defaultPenalty_);
    }

    double getCvarForArea(std::string area) const
    {
        // lower and upper boundaries can be members of the YAMLElement class, if the need for
        // more boundaries arises
        double cvar;
        if (penalties_.count(area) > 0)
        {
            cvar = getValueFromKey<double>(cvarKey, penalties_.at(area));
        }
        else
        {
            cvar = getValueFromKey<double>(cvarKey, defaultPenalty_);
        }
        if (cvar < 0.0 || cvar > 1.0)
        {
            std::cout << "CVaR was read as " << cvar << "; it will be clamped to 0.0 or 1.0."
                      << std::endl;
            cvar = std::max(0.0, std::min(1.0, cvar));
        }
        return cvar;
    }

    int getStartWeek() const
    {
        return getValueFromKey<int>(startWeekKey, elements_);
    }

    int getEndWeek() const
    {
        return getValueFromKey<int>(endWeekKey, elements_);
    }

    int getNbLevels() const
    {
        return getValueFromKey<int>(nbLevelsKey, elements_);
    }

    bool getAntaresFormat() const
    {
        return getValueFromKey<bool>(antaresFormatKey, elements_);
    }

    bool getUseOptimalTrajectory() const
    {
        return getValueFromKey<bool>(useOptimalTrajectoryKey, elements_);
    }

    // keys as used in the YAML file:
    inline static const std::string penaltiesKey = "penalties",
                                    // these are global
      startWeekKey = "start_week", endWeekKey = "end_week", nbLevelsKey = "nb_levels",
                                    antaresFormatKey = "antares_format",
                                    useOptimalTrajectoryKey = "use_optimal_trajectory",
                                    // these will be specific to areas
      penaltyBottomRuleCurveKey = "penalty_bottom_rule_curve",
                                    penaltyUpperRuleCurveKey = "penalty_upper_rule_curve",
                                    penaltyFinalLevelKey = "penalty_final_level",
                                    forceFinalLevelKey = "force_final_level",
                                    finalLevelKey = "final_level", cvarKey = "cvar";

    /// @brief The collection of all penalty-related values, with their YAML key, by area
    std::map<std::string, std::map<std::string, YAMLElement>> penalties_; // area, key, value

    /// @brief A default penalty that is used to get default values if an area was not
    /// present in the YAML file
    std::map<std::string, YAMLElement> defaultPenalty_;

    /// @brief The YAML node being the root of all penalty related values (with key "penalties"), to
    /// be parsed
    YAML::Node penaltiesNode_;

private:
    void emplaceAllElements() override;

    /// @brief Parse the YAML file to find penalties values for all areas
    void parsePenaltiesByArea();

    /// @brief A method returning a properly instantiated collection of penalties-related values,
    /// with their default values
    /// @return
    std::map<std::string, YAMLElement> instantiatePenalty();
};
