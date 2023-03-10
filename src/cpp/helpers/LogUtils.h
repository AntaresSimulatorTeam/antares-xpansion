

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif
#include <string>
class LogUtils {
 public:
  static std::string UserName();
  static std::string HostName();
};