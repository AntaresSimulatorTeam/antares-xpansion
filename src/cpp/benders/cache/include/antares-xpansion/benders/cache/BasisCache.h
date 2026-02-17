#pragma once

#include <optional>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

class BasisCache
{
public:
    using Basis = std::pair<std::vector<int>, std::vector<int>>;
    void set(const std::string& key, const Basis& basis);
    std::optional<Basis> get(const std::string& key) const;
    void serialize(const std::filesystem::path& filename) const;
    void deserialize(const std::filesystem::path& filename);

private:
    std::unordered_map<std::string, Basis> data_;
};

