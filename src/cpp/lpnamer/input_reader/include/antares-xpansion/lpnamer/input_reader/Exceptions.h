#pragma once

class IniFileNotFound : public std::runtime_error {
 public:
  explicit IniFileNotFound(const std::string& msg) : std::runtime_error(msg) {}
};