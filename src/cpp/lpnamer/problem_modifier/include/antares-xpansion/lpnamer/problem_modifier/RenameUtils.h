#pragma once
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class RenameUtils
{
public:
    static bool try_replace_first_token(const std::string& name,
                                        std::string_view prefix,
                                        unsigned int week,
                                        long long factor,
                                        bool ignore_value,
                                        std::string& out);

    std::string replace_hour_in_name(const std::string& name, unsigned int week) const;
    std::pair<const std::vector<std::string>&, const std::vector<std::string>&> rename_week_names(
      unsigned int week,
      const std::vector<std::string>& variables,
      const std::vector<std::string>& contraintes) const;

private:
    void rename_week_names(
      unsigned int week,
      const std::vector<std::string>& names,
      std::unordered_map<int, std::vector<std::string>>& container_names) const;

    using NamesByWeek = std::unordered_map<int, std::vector<std::string>>;
    mutable NamesByWeek variables_names_;
    mutable NamesByWeek constraints_names_;
    mutable std::mutex rename_mutex_;
};
