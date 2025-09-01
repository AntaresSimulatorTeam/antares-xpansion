#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

enum class ProblemsFormat
{
    MPS_FILE,
    OPTIMIZED
};

inline ProblemsFormat problemsFormatFromString(const std::string& str)
{
    auto lower_str = StringManip::StringUtils::ToLowercase(str);
    if (lower_str == "mps")
    {
        return ProblemsFormat::MPS_FILE;
    }
    if (lower_str == "optimized")
    {
        return ProblemsFormat::OPTIMIZED;
    }
    throw std::runtime_error("Unknown ProblemsFormat: " + str);
}
