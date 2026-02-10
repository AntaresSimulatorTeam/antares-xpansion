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
    }

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

    double getCvar() const
    {
        // lower and upper boundaries can be members of the YAMLElement class, if the need for more
        // boundaries arises
        double cvar = getValueFromKey<double>(cvarKey);
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
        return getValueFromKey<int>(startWeekKey);
    }

    int getEndWeek() const
    {
        return getValueFromKey<int>(endWeekKey);
    }

    int getNbLevels() const
    {
        return getValueFromKey<int>(nbLevelsKey);
    }

    bool getAntaresFormat() const
    {
        return getValueFromKey<bool>(antaresFormatKey);
    }

    bool getUseOptimalTrajectory() const
    {
        return getValueFromKey<bool>(useOptimalTrajectoryKey);
    }

private:
    void emplaceAllElements() override;

    // keys from YAML file:
    inline static const std::string penaltyBottomRuleCurveKey = "penalty_bottom_rule_curve",
                                    penaltyUpperRuleCurveKey = "penalty_upper_rule_curve",
                                    penaltyFinalLevelKey = "penalty_final_level",
                                    forceFinalLevelKey = "force_final_level",
                                    finalLevelKey = "final_level", cvarKey = "cvar",
                                    startWeekKey = "start_week", endWeekKey = "end_week",
                                    nbLevelsKey = "nb_levels", antaresFormatKey = "antares_format",
                                    useOptimalTrajectoryKey = "use_optimal_trajectory";
};
