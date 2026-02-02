#pragma once

#include <fmt/format.h>
#include <istream>
#include <ostream>
#include <string>

#include "antares-xpansion/core/ProblemFormat.h"

inline std::ostream& operator<<(std::ostream& stream, const ProblemsFormat& rhs)
{
    switch (rhs)
    {
    case ProblemsFormat::MPS_FILE:
        stream << "MPS";
        break;
    case ProblemsFormat::OPTIMIZED:
        stream << "OPTIMIZED";
        break;
    default:
        stream << "Unknown";
    }
    return stream;
}

inline std::istream& operator>>(std::istream& stream, ProblemsFormat& rhs)
{
    std::string str;
    stream >> str;
    if (stream)
    {
        try
        {
            rhs = problemsFormatFromString(str);
        }
        catch (const std::runtime_error&)
        {
            stream.setstate(std::ios::failbit);
            throw;
        }
    }
    return stream;
}

template<>
struct fmt::formatter<ProblemsFormat>: formatter<string_view>
{
    // parse is inherited from formatter<string_view>.

    auto format(ProblemsFormat problems_format, format_context& ctx) const
      -> format_context::iterator;
};
