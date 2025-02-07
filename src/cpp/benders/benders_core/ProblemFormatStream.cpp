#include "antares-xpansion/benders/benders_core/ProblemFormatStream.h"

auto fmt::formatter<ProblemsFormat>::format(ProblemsFormat problems_format,
                                            format_context& ctx) const -> format_context::iterator
{
    string_view result = "Unknown";
    switch (problems_format)
    {
    case ProblemsFormat::MPS_FILE:
        result = "MPS";
        break;
    case ProblemsFormat::SAVED_FILE:
        result = "SAVED";
        break;
    default:
        result = "Unknown";
    }
    return formatter<string_view>::format(result, ctx);
}
