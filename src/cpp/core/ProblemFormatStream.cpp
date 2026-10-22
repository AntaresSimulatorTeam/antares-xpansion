#include "antares-xpansion/core/ProblemFormatStream.h"

auto fmt::formatter<ProblemFormat>::format(ProblemFormat problem_format,
                                           format_context& ctx) const -> format_context::iterator
{
    string_view result = "Unknown";
    switch (problem_format)
    {
    case ProblemFormat::MPS_FILE:
        result = "MPS";
        break;
    case ProblemFormat::OPTIMIZED:
        result = "OPTIMIZED";
        break;
    default:
        result = "Unknown";
    }
    return formatter<string_view>::format(result, ctx);
}
