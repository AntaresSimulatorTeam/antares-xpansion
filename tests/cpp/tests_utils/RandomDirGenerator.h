#ifndef __RANDOMDIRGENERATOR_H_
#define __RANDOMDIRGENERATOR_H_
#include <time.h>

#include <filesystem>
#include <string>

std::string timeToStr(const std::time_t& time_p);

std::string GenerateRandomString(size_t len);
std::filesystem::path createRandomSubDir(
    const std::filesystem::path& parentDir);
#endif  //__RANDOMDIRGENERATOR_H_