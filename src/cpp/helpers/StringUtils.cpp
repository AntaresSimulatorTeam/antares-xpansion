#include <algorithm>
#include <iterator>

#include "include/helpers/StringUtils.h"


std::string StringUtils::ToLowercase(const std::string& s)
{
    std::string result;
    std::transform(s.cbegin(), s.cend(), std::back_inserter(result), [](char const& c) {
        return std::tolower(c);
    });
    return result;
}
