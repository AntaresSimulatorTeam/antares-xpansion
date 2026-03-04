#include "include/antares-xpansion/bellman_values/SettingsConfigReader.h"

void SettingsConfigReader::emplaceAllElements()
{
    // instantiate all elements with key (as in YAML), default value, and boolean for optional
    // type is inferred from default value
    elements_.emplace(solverKey, YAMLElement(solverKey, "xpress"));
    elements_.emplace(keepMpsKey, YAMLElement(keepMpsKey, false));
    elements_.emplace(problemFormatKey, YAMLElement(problemFormatKey, "OPTIMIZED"));
    elements_.emplace(verbosityKey, YAMLElement(verbosityKey, "INFO"));
    elements_.emplace(cacheProblemsKey, YAMLElement(cacheProblemsKey, false));
}
