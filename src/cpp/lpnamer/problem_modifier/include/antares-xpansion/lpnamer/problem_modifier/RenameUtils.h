#pragma once
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class RenameUtils
{
public:
    bool try_replace_first_token(const std::string& name,
                                 std::string_view prefix,
                                 unsigned int week,
                                 long long factor,
                                 bool ignore_value,
                                 std::string& out) const;

    std::string replace_hour_in_name(const std::string& name, unsigned int week) const;
    std::pair<std::vector<std::string>&, std::vector<std::string>&> rename_week_names(
      unsigned int week,
      const std::vector<std::string>& variables,
      const std::vector<std::string>& contraintes) const;

    void rename_week_names(
      unsigned int week,
      const std::vector<std::string>& names,
      std::unordered_map<int, std::vector<std::string>>& container_names) const;

private:
    mutable std::unordered_map<int, std::vector<std::string>> variables_names;
    mutable std::unordered_map<int, std::vector<std::string>> constraints_names;
    mutable std::mutex rename_mutex;
};
