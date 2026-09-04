#pragma once

#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <string>

#include "antares-xpansion/xpansion_interfaces/StringManip.h"

enum class CacheProblems
{
    NO_CACHE,
    PER_SUB,
    COMPACT
};

inline CacheProblems cacheProblemsFromString(const std::string& str)
{
    auto lower_str = StringManip::StringUtils::ToLowercase(str);
    if (lower_str == "no_cache" || lower_str == "0")
    {
        return CacheProblems::NO_CACHE;
    }
    if (lower_str == "per_sub" || lower_str == "1")
    {
        return CacheProblems::PER_SUB;
    }
    if (lower_str == "compact" || lower_str == "2")
    {
        return CacheProblems::COMPACT;
    }
    throw std::runtime_error("Unknown CacheProblems: " + str);
}

inline std::ostream& operator<<(std::ostream& stream, const CacheProblems& rhs)
{
    switch (rhs)
    {
    case CacheProblems::NO_CACHE:
        stream << "NO_CACHE";
        break;
    case CacheProblems::PER_SUB:
        stream << "PER_SUB";
        break;
    case CacheProblems::COMPACT:
        stream << "COMPACT";
        break;
    default:
        stream << "Unknown";
    }
    return stream;
}
