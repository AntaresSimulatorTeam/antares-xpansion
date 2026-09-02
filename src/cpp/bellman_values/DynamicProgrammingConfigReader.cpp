#include "include/antares-xpansion/bellman_values/DynamicProgrammingConfigReader.h"

void DynamicProgrammingConfigReader::emplaceAllElements()
{
    // instantiate all elements with key (as in YAML), default value, and boolean for optional
    // type is inferred from default value
    // These are global level parameters, as in, not specific to any area
    elements_.emplace(startWeekKey, YAMLElement(startWeekKey, 1));
    elements_.emplace(endWeekKey, YAMLElement(endWeekKey, 52));
    elements_.emplace(nbLevelsKey, YAMLElement(nbLevelsKey, 10));
    elements_.emplace(antaresFormatKey, YAMLElement(antaresFormatKey, false));
    elements_.emplace(useOptimalTrajectoryKey, YAMLElement(useOptimalTrajectoryKey, false));
}

void DynamicProgrammingConfigReader::parsePenaltiesByArea()
{
    // this method assumes that config_ has been filled previously
    if (config_.size() > 0 && config_[penaltiesKey].size() > 0)
    {
        std::cout << "Parsing penalties:" << std::endl;
        // there's a penalties section in the file, that can be parsed
        for (auto penaltiesNode: config_[penaltiesKey])
        {
            if (penaltiesNode.second)
            {
                std::string areaName = penaltiesNode.first.as<std::string>();
                std::cout << "Parsing penalty for area " << areaName << std::endl;
                // instantiating penalty with default values
                std::map<std::string, YAMLElement> penalty = instantiatePenalty();
                // update with values read in file
                for (auto& [key, element]: penalty)
                {
                    element.updateValue(penaltiesNode.second);
                    std::cout << "Parsing value for " << key << " for area " << areaName
                              << std::endl;
                }

                // add to stored penalties
                penalties_.emplace(areaName, penalty);
            }
            else
            {
                std::cout << "No penalty for area " << penaltiesNode.first.as<std::string>()
                          << std::endl;
            }
        }
    }
    else
    {
        // either no file or no penalties section
        std::cout << "Could not parse penalties in YAML file. Default values will be used."
                  << std::endl;
    }
    // a default penalty must be instantiated in case a required area is not present in the file
    defaultPenalty_ = instantiatePenalty();
}

std::map<std::string, ConfigReader::YAMLElement>
DynamicProgrammingConfigReader::instantiatePenalty()
{
    std::map<std::string, YAMLElement> penalty;
    // this is where default penalty values are defined
    penalty.emplace(penaltyBottomRuleCurveKey, YAMLElement(penaltyBottomRuleCurveKey, 0.0));
    penalty.emplace(penaltyUpperRuleCurveKey, YAMLElement(penaltyUpperRuleCurveKey, 0.0));
    penalty.emplace(penaltyFinalLevelKey, YAMLElement(penaltyFinalLevelKey, 0.0));
    penalty.emplace(forceFinalLevelKey, YAMLElement(forceFinalLevelKey, false));
    penalty.emplace(finalLevelKey,
                    YAMLElement(finalLevelKey,
                                std::optional<double>{}, // will default to initial level later in
                                // code, if empty
                                true));
    penalty.emplace(cvarKey, YAMLElement(cvarKey,
                                         1.0)); // default: take all scenarios into account
    return penalty;
}
