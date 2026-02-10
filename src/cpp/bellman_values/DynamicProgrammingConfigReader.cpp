#include "include/antares-xpansion/bellman_values/DynamicProgrammingConfigReader.h"

void DynamicProgrammingConfigReader::emplaceAllElements()
{
    // instantiate all elements with key (as in YAML), default value, and boolean for optional
    // type is inferred from default value
    elements_.emplace(penaltyBottomRuleCurveKey, YAMLElement(penaltyBottomRuleCurveKey, 0.0));
    elements_.emplace(penaltyUpperRuleCurveKey, YAMLElement(penaltyUpperRuleCurveKey, 0.0));
    elements_.emplace(penaltyFinalLevelKey, YAMLElement(penaltyFinalLevelKey, 0.0));
    elements_.emplace(forceFinalLevelKey, YAMLElement(forceFinalLevelKey, false));
    elements_.emplace(
      finalLevelKey,
      YAMLElement(finalLevelKey,
                  std::optional<double>{}, // will default to initial level later in code, if empty
                  true));
    elements_.emplace(cvarKey,
                      YAMLElement(cvarKey, 1.0)); // default: take all scenarios into account
    elements_.emplace(startWeekKey, YAMLElement(startWeekKey, 1));
    elements_.emplace(endWeekKey, YAMLElement(endWeekKey, 52));
    elements_.emplace(nbLevelsKey, YAMLElement(nbLevelsKey, 10));
    elements_.emplace(antaresFormatKey, YAMLElement(antaresFormatKey, false));
    elements_.emplace(useOptimalTrajectoryKey, YAMLElement(useOptimalTrajectoryKey, false));
}
