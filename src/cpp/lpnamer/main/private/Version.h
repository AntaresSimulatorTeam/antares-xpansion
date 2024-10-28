//
// Created by marechaljas on 10/01/24.
//

#pragma once
#include <string_view>

struct Version {
  explicit Version(std::string_view version);

  bool operator<(const Version& another) const;

  bool operator>(const Version& another) const;

  bool operator==(const Version& another) const;
  bool operator<=(const Version& another) const;
  bool operator>=(const Version& another) const;

 private:
  int major;
  int minor;
};