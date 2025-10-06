#include <algorithm>
#include <charconv>
#include <mutex>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

constexpr unsigned int HOURS_IN_A_WEEK = 168;
constexpr unsigned int DAYS_IN_A_WEEK = 7;

extern std::unordered_map<int, std::vector<std::string>> variables_names;
extern std::unordered_map<int, std::vector<std::string>> constraints_names;
extern std::mutex rename_mutex;

static bool try_replace_first_token(const std::string& name,
                                    std::string_view prefix,
                                    unsigned int week,
                                    long long factor,
                                    bool ignore_value,
                                    std::string& out)
{
    const auto pos = name.find(prefix);
    if (pos == std::string::npos)
    {
        return false;
    }

    const auto start = pos + prefix.size();
    const auto end = name.find('>', start);
    if (end == std::string::npos)
    {
        throw std::runtime_error(LOGLOCATION + std::string("Malformed token: missing '>'"));
    }

    long long original = 0;
    if (!ignore_value)
    {
        const auto digits = name.substr(start, end - start);
        auto first = digits.data();
        auto last = digits.data() + digits.size();
        auto ec = std::from_chars(first, last, original);
        if (ec.ec != std::errc() || ec.ptr != last)
        {
            throw std::runtime_error(LOGLOCATION
                                     + std::string("Malformed token: non-integer value"));
        }
    }

    const long long new_value = ignore_value
                                  ? static_cast<long long>(week) - 1
                                  : (static_cast<long long>(week) - 1) * factor + original;

    out = name;
    out.replace(pos, (end - pos) + 1, std::string(prefix) + std::to_string(new_value) + ">");
    return true;
}

static std::string replace_hour_in_name(const std::string& name, unsigned int week)
{
    if (week == 0)
    {
        throw std::invalid_argument(LOGLOCATION + std::string("week must be >= 1"));
    }

    std::string out;
    if (try_replace_first_token(name, "hour<", week, HOURS_IN_A_WEEK, false, out))
    {
        return out;
    }
    if (try_replace_first_token(name, "day<", week, DAYS_IN_A_WEEK, false, out))
    {
        return out;
    }
    if (try_replace_first_token(name, "week<", week, 1, true, out))
    {
        return out;
    }

    throw std::runtime_error(LOGLOCATION + std::string("No [hour|day|week]<...> pattern found in ")
                             + name);
}

void rename_week_names(unsigned int week,
                       const std::vector<std::string>& names,
                       std::unordered_map<int, std::vector<std::string>>& container_names)
{
    /* The numbering is the same for every week N of each year. We only need to compute
     * the renaming once for each week N.
     */
    if (!container_names.contains(week))
    {
        std::vector<std::string> renamed_variables;
        renamed_variables.reserve(names.size());
        std::ranges::transform(names,
                               std::back_inserter(renamed_variables),
                               [&week](const auto& n) { return replace_hour_in_name(n, week); });
        std::lock_guard guard(rename_mutex);
        container_names.emplace(week, renamed_variables);
    }
}
