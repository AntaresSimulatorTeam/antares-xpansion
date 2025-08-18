#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>

enum class ProblemsFormat
{
    MPS_FILE,
    SAVED_FILE
};

inline ProblemsFormat problemsFormatFromString(const std::string& str)
{
    auto lower_str = str;
    std::transform(str.begin(), str.end(), lower_str.begin(), ::tolower);
    if (lower_str == "mps")
    {
        return ProblemsFormat::MPS_FILE;
    }
    else if (lower_str == "saved")
    {
        return ProblemsFormat::SAVED_FILE;
    }
    else
    {
        throw std::runtime_error("Unknown ProblemsFormat: " + str);
    }
}
