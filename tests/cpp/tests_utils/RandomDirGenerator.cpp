#include "RandomDirGenerator.h"

#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#endif

std::string timeToStr(const std::time_t& time_p) {
  struct tm local_time;
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  localtime_s(&local_time, &time_p);
#else  // defined(__unix__) || (__APPLE__)
  localtime_r(&time_p, &local_time);
#endif
  const char* FORMAT = "%d-%m-%Y-%H-%M-%S";
  char buffer_l[100];
  strftime(buffer_l, sizeof(buffer_l), FORMAT, &local_time);

  return buffer_l;
}

std::string GenerateRandomString(size_t len) {
  srand(time(NULL));
  const auto abc = "abcdefghijklmnopqrstuvwxyz";

  std::string ret = "XXXXXX";

  for (int i = 0; i < len; ++i) {
    ret[i] = abc[rand() % (sizeof(abc) - 1)];
  }
  return ret;
}
std::filesystem::path GetRandomSubDirPath(
    const std::filesystem::path& parentDir) {
  return parentDir /
         (timeToStr(std::time(nullptr)) + "-" + GenerateRandomString(6));
}
std::filesystem::path CreateRandomSubDir_(
    const std::filesystem::path& parentDir) {
  const auto subDirPath = GetRandomSubDirPath(parentDir);
  std::filesystem::create_directory(subDirPath);
  return subDirPath;
}
std::filesystem::path CreateRandomSubDir(
    const std::filesystem::path& parentDir) {
  char template_array[] = "XXXXXX";
#ifdef __linux__
  auto template_dir = parentDir / template_array;
  char* template_dir_array = template_dir.string().data();
  if (auto ret = mkdtemp(template_dir_array); ret != nullptr) {
    return ret;
  }
#elif _WIN32
  if (auto ret = _mktemp_s(template_array); ret == 0) {
    auto created_dir = parentDir / template_array;
    std::filesystem::create_directory(created_dir);
    return created_dir;
  }
#endif
  else {
    return CreateRandomSubDir_(parentDir);
  }
}
