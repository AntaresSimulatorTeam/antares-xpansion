#pragma once
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Fonctions utilitaires pour le renommage des variables et contraintes

extern std::unordered_map<int, std::vector<std::string>> variables_names;
extern std::unordered_map<int, std::vector<std::string>> constraints_names;
extern std::mutex rename_mutex;

bool try_replace_first_token(const std::string& name,
                             std::string_view prefix,
                             unsigned int week,
                             long long factor,
                             bool ignore_value,
                             std::string& out);

std::string replace_hour_in_name(const std::string& name, unsigned int week);

void rename_week_names(unsigned int week,
                       const std::vector<std::string>& names,
                       std::unordered_map<int, std::vector<std::string>>& container_names);
