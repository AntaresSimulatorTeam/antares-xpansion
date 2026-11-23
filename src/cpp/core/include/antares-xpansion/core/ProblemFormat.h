#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

enum class ProblemFormat
{
    MPS_FILE,
    OPTIMIZED
};

inline ProblemFormat problemFormatFromString(const std::string& str)
{
    auto lower_str = StringManip::StringUtils::ToLowercase(str);
    if (lower_str == "mps")
    {
        return ProblemFormat::MPS_FILE;
    }
    if (lower_str == "optimized")
    {
        return ProblemFormat::OPTIMIZED;
    }
    throw std::runtime_error("Unknown ProblemFormat: " + str);
}
