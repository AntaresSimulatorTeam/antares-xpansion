//
// Created by marechaljas on 10/01/24.
//

#include "Version.h"

#include "StringManip.h"

Version::Version(std::string_view version) {
  auto split_version = StringManip::split(StringManip::trim(version), '.');
  major = std::stoi(split_version[0]);
  minor = std::stoi(split_version[1]);
}
bool Version::operator<(const Version& another) const {
  if (this->major == another.major) {
    return this->minor < another.minor;
  } else {
    return this->major < another.major;
  }
}
bool Version::operator>(const Version& another) const {
  return !operator<(another);
}
bool Version::operator==(const Version& another) const {
  return (this->major == another.major && this->minor == another.minor);
}
bool Version::operator<=(const Version& another) const {
  return (operator<(another) || operator==(another));
}
bool Version::operator>=(const Version& another) const {
  return (operator>(another) || operator==(another));
}
