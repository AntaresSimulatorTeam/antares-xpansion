#pragma once
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Classe utilitaire pour le renommage des variables et contraintes
class RenameUtils
{
public:
    static bool try_replace_first_token(const std::string& name,
                                        std::string_view prefix,
                                        unsigned int week,
                                        long long factor,
                                        bool ignore_value,
                                        std::string& out);

    static std::string replace_hour_in_name(const std::string& name, unsigned int week);

    static void rename_week_names(
      unsigned int week,
      const std::vector<std::string>& names,
      std::unordered_map<int, std::vector<std::string>>& container_names);

    // Accès aux conteneurs de renommage (si besoin)
    static std::unordered_map<int, std::vector<std::string>>& get_variables_names();
    static std::unordered_map<int, std::vector<std::string>>& get_constraints_names();
    static std::mutex& get_rename_mutex();

private:
    static std::unordered_map<int, std::vector<std::string>> variables_names;
    static std::unordered_map<int, std::vector<std::string>> constraints_names;
    static std::mutex rename_mutex;
};
