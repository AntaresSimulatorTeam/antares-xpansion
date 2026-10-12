#pragma once

#include <ostream>
#include <stdexcept>
#include <string>

enum class RestartMode
{
    COLD_START = 0,
    RESUME = 1,
    HOT_START = 2
};

inline RestartMode restartModeFromString(const std::string& str)
{
    if (str == "cold_start")
    {
        return RestartMode::COLD_START;
    }
    if (str == "resume")
    {
        return RestartMode::RESUME;
    }
    if (str == "hot_start")
    {
        return RestartMode::HOT_START;
    }
    throw std::runtime_error("Unknown RestartMode: " + str
                             + " (expected cold_start, resume, or hot_start)");
}

// True when the run continues an interrupted one (append logs, reload previous output state)
inline bool IsResume(RestartMode mode)
{
    return mode == RestartMode::RESUME;
}

// True when the master is read from an existing file (resume and hot start)
inline bool UsesExistingMaster(RestartMode mode)
{
    return mode != RestartMode::COLD_START;
}

inline std::ostream& operator<<(std::ostream& stream, const RestartMode& mode)
{
    switch (mode)
    {
    case RestartMode::COLD_START:
        stream << "cold_start";
        break;
    case RestartMode::RESUME:
        stream << "resume";
        break;
    case RestartMode::HOT_START:
        stream << "hot_start";
        break;
    }
    return stream;
}
