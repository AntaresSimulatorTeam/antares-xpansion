#include "LogUtils.h"
#define INFO_BUFFER_SIZE 32767

std::string LogUtils::UserName() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR user_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  GetUserName(user_name, &bufCharCount);
  return user_name;
#else
  char user_name[LOGIN_NAME_MAX];
  getlogin_r(user_name, LOGIN_NAME_MAX);
  return user_name;
#endif
}
std::string LogUtils::HostName() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  TCHAR host_name[INFO_BUFFER_SIZE];
  DWORD bufCharCount = INFO_BUFFER_SIZE;

  GetComputerName(host_name, &bufCharCount);
  return host_name;
#else
  char host_name[HOST_NAME_MAX];
  gethostname(host_name, HOST_NAME_MAX);
  return host_name;
#endif
}