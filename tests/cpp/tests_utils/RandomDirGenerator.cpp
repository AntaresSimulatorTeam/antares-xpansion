#include "RandomDirGenerator.h"
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
std::filesystem::path CreateRandomSubDir(
    const std::filesystem::path& parentDir) {
  const auto subDirPath = GetRandomSubDirPath(parentDir);
  std::filesystem::create_directory(subDirPath);
  return subDirPath;
}
