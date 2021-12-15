#ifndef ANTARESXPANSION_PATH_H
#define ANTARESXPANSION_PATH_H
#include <iostream>
#include <string>

class Path {
private:
  std::string mPath;

public:
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  // Windows
  static const char mSep = '\\';
#else
  // Linux, Unix, Apple
  static const char mSep = '/';
#endif
public:
  Path(const std::string& s) : mPath(s){}
  Path(std::string&& s) : mPath(std::move(s)){}
  Path operator/(const Path& other) {
    return Path(mPath + mSep + other.mPath);
  }
  Path operator/(const std::string& s) {
    return Path(mPath + mSep + s);
  }
  operator std::string() const {
    return mPath;
  }
  friend std::ostream& operator<<(std::ostream& os, const Path& p) {
    os << p.mPath;
    return os;
  }
};

#endif // ANTARESXPANSION_PATH_H