#include "antares-xpansion/lpnamer/problem_modifier/RenameUtils.h"

#include <algorithm>
#include <charconv>
#include <mutex>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h"
#include "antares-xpansion/xpansion_interfaces/LogUtils.h"

constexpr unsigned int HOURS_IN_A_WEEK = 168;
constexpr unsigned int DAYS_IN_A_WEEK = 7;

bool RenameUtils::try_replace_first_token(const std::string& name,
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

std::optional<std::string> RenameUtils::replace_time_step_in_name(const std::string& name,
                                                                  unsigned int week)
{
    if (week == 0)
    {
        return std::nullopt;
    }

    std::string out;
    if (try_replace_first_token(name, "hour<~", week, HOURS_IN_A_WEEK, false, out))
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

    return std::nullopt;
}

std::optional<std::pair<const std::vector<std::string>&, const std::vector<std::string>&>>
RenameUtils::rename_week_names(unsigned int week,
                               const std::vector<std::string>& variables,
                               const std::vector<std::string>& constraints,
                               ProblemGenerationLog::ProblemGenerationLogger* logger) const
{
    if (!rename_week_names(week, variables, variables_names_, logger))
    {
        return std::nullopt;
    }
    if (!rename_week_names(week, constraints, constraints_names_, logger))
    {
        return std::nullopt;
    }
    return std::make_pair(std::ref(variables_names_.at(week)),
                          std::ref(constraints_names_.at(week)));
}

bool RenameUtils::rename_week_names(
  unsigned int week,
  const std::vector<std::string>& names,
  std::unordered_map<int, std::vector<std::string>>& container_names,
  ProblemGenerationLog::ProblemGenerationLogger* logger) const
{
    if (!container_names.contains(week))
    {
        std::vector<std::string> renamed_variables;
        renamed_variables.reserve(names.size());
        for (const auto& n: names)
        {
            auto renamed = replace_time_step_in_name(n, week);
            if (!renamed)
            {
                if (logger)
                {
                    (*logger)(LogUtils::LOGLEVEL::ERR) << "Failed to rename variable/constraint '"
                                                       << n << "' for week " << week << std::endl;
                }
                return false;
            }
            renamed_variables.push_back(*renamed);
        }
        std::lock_guard guard(rename_mutex_);
        container_names.emplace(week, renamed_variables);
    }
    return true;
}
