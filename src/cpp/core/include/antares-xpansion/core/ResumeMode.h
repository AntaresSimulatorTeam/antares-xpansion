#pragma once

#include <ostream>
#include <stdexcept>
#include <string>

enum class ResumeMode
{
    COLD_START = 0,
    RESUME = 1,
    HOT_START = 2
};

inline ResumeMode resumeModeFromString(const std::string& str)
{
    if (str == "0" || str == "cold_start")
    {
        return ResumeMode::COLD_START;
    }
    if (str == "1" || str == "resume")
    {
        return ResumeMode::RESUME;
    }
    if (str == "2" || str == "hot_start")
    {
        return ResumeMode::HOT_START;
    }
    throw std::runtime_error("Unknown ResumeMode: " + str
                             + " (expected 0, 1, 2, cold_start, resume, or hot_start)");
}

inline std::ostream& operator<<(std::ostream& stream, const ResumeMode& mode)
{
    switch (mode)
    {
    case ResumeMode::COLD_START:
        stream << "cold_start";
        break;
    case ResumeMode::RESUME:
        stream << "resume";
        break;
    case ResumeMode::HOT_START:
        stream << "hot_start";
        break;
    }
    return stream;
}
